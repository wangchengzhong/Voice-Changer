/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Rubber Band Library
    An audio time-stretching and pitch-shifting library.
    Copyright 2007-2021 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for the
    Rubber Band Library obtained by agreement with the copyright
    holders, you may redistribute and/or modify it under the terms
    described in that licence.

    If you wish to distribute code using the Rubber Band Library
    under terms other than those of the GNU General Public License,
    you must obtain a valid commercial licence before doing so.
*/
#include"JuceHeader.h"
#include "StretcherImpl.h"

#include "audiocurves/PercussiveAudioCurve.h"
#include "audiocurves/HighFrequencyAudioCurve.h"
#include "audiocurves/ConstantAudioCurve.h"

#include "StretchCalculator.h"
#include "StretcherChannelData.h"

#include "dsp/Resampler.h"
#include "base/Profiler.h"
#include "system/VectorOps.h"
#include "system/sysutils.h"

#include <cassert>
#include <cmath>
#include <set>
#include <map>
#include <deque>
#include <algorithm>

using namespace RubberBand;

using std::cerr;
using std::endl;

namespace RubberBand {

#ifndef NO_THREADING

RubberBandStretcher::Impl::ProcessThread::ProcessThread(Impl *s, size_t c) :
    m_s(s),
    m_channel(c),
    m_dataAvailable(std::string("data ") + char('A' + c)),
    m_abandoning(false)
{ }

void
RubberBandStretcher::Impl::ProcessThread::run()
{
    if (m_s->m_debugLevel > 1) {
        cerr << "thread " << m_channel << " getting going" << endl;
    }

    ChannelData &cd = *m_s->m_channelData[m_channel];

    while (cd.inputSize == -1 ||
           cd.inbuf->getReadSpace() > 0) {

//        if (cd.inputSize != -1) {
//            cerr << "inputSize == " << cd.inputSize
//                 << ", readSpace == " << cd.inbuf->getReadSpace() << endl;
//        }
        
        bool any = false, last = false;
        m_s->processChunks(m_channel, any, last);
        // cerr << "have run at this point, in stretcherProcess.cpp, bool any = false, last = false";
        if (last) break;

        if (any) {
            m_s->m_spaceAvailable.lock();
            m_s->m_spaceAvailable.signal();
            m_s->m_spaceAvailable.unlock();
        }

        m_dataAvailable.lock();
        if (!m_s->testInbufReadSpace(m_channel) && !m_abandoning) {
            m_dataAvailable.wait(50000); // bounded in case of abandonment
        }
        m_dataAvailable.unlock();

        if (m_abandoning) 
        {
            return;
        }
    }

    bool any = false, last = false;
    m_s->processChunks(m_channel, any, last);
    m_s->m_spaceAvailable.lock();
    m_s->m_spaceAvailable.signal();
    m_s->m_spaceAvailable.unlock();
    
    if (m_s->m_debugLevel > 1) {
        cerr << "thread " << m_channel << " done" << endl;
    }
}

void RubberBandStretcher::Impl::ProcessThread::signalDataAvailable()
{
    m_dataAvailable.lock();
    m_dataAvailable.signal();
    m_dataAvailable.unlock();
}

void RubberBandStretcher::Impl::ProcessThread::abandon()
{
    m_abandoning = true;
}

#endif

bool RubberBandStretcher::Impl::resampleBeforeStretching() const
{
    // We can't resample before stretching in offline mode, because
    // the stretch calculation is based on doing it the other way
    // around.  It would take more work (and testing) to enable this.
    if (!m_realtime) return false;

    if (m_options & OptionPitchHighQuality)// equal to &&
    {
        return (m_outputPitchRatio < 1.0); // better sound
    } 
    else if (m_options & OptionPitchHighConsistency) 
    {
        return false;
    } 
    else 
    {
        return (m_outputPitchRatio > 1.0); // better performance
    }
}

void RubberBandStretcher::Impl::prepareChannelMS(size_t c,
                                            const float *const *inputs,
                                            size_t offset,
                                            size_t samples, 
                                            float *prepared)
{
    for (size_t i = 0; i < samples; ++i) {
        float left = inputs[0][i + offset];
        float right = inputs[1][i + offset];
        float mid = (left + right) / 2;
        float side = (left - right) / 2;
        if (c == 0) {
            prepared[i] = mid;
        } else {
            prepared[i] = side;
        }
    }
}
    
size_t RubberBandStretcher::Impl::consumeChannel(size_t c,
                                          const float *const *origInputBuffer,
                                          size_t offset,
                                          size_t samples,
                                          bool final)
{
    Profiler profiler("RubberBandStretcher::Impl::consumeChannel");

    ChannelData &cd = *m_channelData[c];
    RingBuffer<float> &inbuf = *cd.inbuf;

    size_t toWriteSampleNum = samples;
    size_t writableSampleNum = inbuf.getWriteSpace();
    // DBG(writableSampleNum);//2176(highest) 2503(low)
    // change with ratio, but always the same other times
    bool shouldResampleBeforeStretching = resampleBeforeStretching();
    //always false

    const float *actualPutToInbuf = 0;

    bool useMidSide = ((m_options & OptionChannelsTogether) &&
                       (m_channels >= 2) &&
                       (c < 2));
    //if (useMidSide) DBG("use MID side");
    // never use mide side
    if (shouldResampleBeforeStretching) 
    {
        // DBG("resample");
        // never run here
        Profiler profiler2("RubberBandStretcher::Impl::resample");
        
        toWriteSampleNum = int(ceil(samples / m_outputPitchRatio));
        if (writableSampleNum < toWriteSampleNum) 
        {
            samples = int(floor(writableSampleNum * m_outputPitchRatio));
            if (samples == 0) return 0;
        }

        size_t reqSize = int(ceil(samples / m_outputPitchRatio));
        if (reqSize > cd.resamplebufSize) 
        {
            cerr << "WARNING: RubberBandStretcher::Impl::consumeChannel: resizing resampler buffer from "
                 << cd.resamplebufSize << " to " << reqSize << endl;
            cd.setResampleBufSize(reqSize);
        }

#ifndef NO_THREADING
#if defined HAVE_IPP && !defined USE_SPEEX
        if (m_threaded) {
            m_resamplerMutex.lock();
        }
#endif
#endif

        if (useMidSide) 
        {
            prepareChannelMS(c, origInputBuffer, offset, samples, cd.ms);
            actualPutToInbuf = cd.ms;
        } 
        else 
        {
            actualPutToInbuf = origInputBuffer[c] + offset;
        }

        toWriteSampleNum = cd.resampler->resample(&cd.resamplebuf,
                                         cd.resamplebufSize,
                                         &actualPutToInbuf,
                                         samples,
                                         1.0 / m_outputPitchRatio,
                                         final);

#ifndef NO_THREADING
#if defined HAVE_IPP && !defined USE_SPEEX
        if (m_threaded) {
            m_resamplerMutex.unlock();
        }
#endif
#endif
    }

    if (writableSampleNum < toWriteSampleNum) 
    {
        // never run here
        // DBG("run here");
        if (shouldResampleBeforeStretching) 
        {
            return 0;
        }
        toWriteSampleNum = writableSampleNum;
        // DBG("towrite:" << toWriteSampleNum);
        // never run here
    }

    if (shouldResampleBeforeStretching) 
    {
        // DBG("run here"); // never run here
        inbuf.write(cd.resamplebuf, toWriteSampleNum);
        cd.into_RB_InbufSampleCount += samples;
        return samples;
    } 
    else 
    {

        if (useMidSide) 
        {
            // never run here
            prepareChannelMS(c, origInputBuffer, offset, toWriteSampleNum, cd.ms);
            actualPutToInbuf = cd.ms;
        } 
        else 
        {
            actualPutToInbuf = origInputBuffer[c] + offset;
            // DBG(input[c]); // actual sample
        }

        inbuf.write(actualPutToInbuf, toWriteSampleNum);
        cd.into_RB_InbufSampleCount += toWriteSampleNum;
        return toWriteSampleNum;
    }
}

void RubberBandStretcher::Impl::processChunks(size_t c, bool &any, bool &last)
{
    Profiler profiler("RubberBandStretcher::Impl::processChunks");

    // Process as many chunks as there are available on the input
    // buffer for channel c.  This requires that the increments have
    // already been calculated.

    // This is the normal process method in offline mode.

    ChannelData &cd = *m_channelData[c];

    last = false;
    any = false;

    float *tmp = 0;

    while (!last) {

        if (!testInbufReadSpace(c)) {
            if (m_debugLevel > 1) {
                cerr << "processChunks: out of input" << endl;
            }
            break;
        }

        any = true;

        if (!cd.draining) {
            size_t ready = cd.inbuf->getReadSpace();
            assert(ready >= m_aWindowSize || cd.inputSize >= 0);
            cd.inbuf->sendToFft(cd.fltbuf, std::min(ready, m_aWindowSize));
            cd.inbuf->skip(m_inbufJumpSampleNum);
        }

        bool shouldResetPhase = false;
        size_t phaseIncrement, shiftIncrement;
        getIncrements(c, phaseIncrement, shiftIncrement, shouldResetPhase);

        if (shiftIncrement <= m_aWindowSize) {
            windowingAndFft(c);
            last = processChunkForChannel
                (c, phaseIncrement, shiftIncrement, shouldResetPhase);
        } else {
            size_t bit = m_aWindowSize/4;
            if (m_debugLevel > 1)
            {
                cerr << "channel " << c << " breaking down overlong increment " << shiftIncrement << " into " << bit << "-size bits" << endl;
            }
            if (!tmp) tmp = allocate<float>(m_aWindowSize);
            windowingAndFft(c);
            v_copy(tmp, cd.fltbuf, m_aWindowSize);
            for (size_t i = 0; i < shiftIncrement; i += bit) 
            {
                v_copy(cd.fltbuf, tmp, m_aWindowSize);
                size_t thisIncrement = bit;
                if (i + thisIncrement > shiftIncrement) 
                {
                    thisIncrement = shiftIncrement - i;
                }
                last = processChunkForChannel
                    (c, phaseIncrement + i, thisIncrement, shouldResetPhase);
                shouldResetPhase = false;
            }
        }

        cd.chunkSampleCount++;
        if (m_debugLevel > 2) 
        {
            cerr << "channel " << c << ": last = " << last << ", chunkSampleCount = " << cd.chunkSampleCount << endl;
        }
    }

    if (tmp) deallocate(tmp);
}

bool RubberBandStretcher::Impl::processOneChunk()
{
    Profiler profiler("RubberBandStretcher::Impl::processOneChunk");

    // Process a single chunk for all channels, provided there is
    // enough data on each channel for at least one chunk.  This is
    // able to calculate increments as it goes along.

    // This is the normal process method in RT mode.

    for (size_t ch = 0; ch < m_channels; ++ch) 
    {
        if (!testInbufReadSpace(ch)) 
        {
            if (m_debugLevel > 1) 
            {
                // never run here
                // DBG("processOneChunk: out of input\n");
            }
            return false;
        }
        //DBG("have here"); run here every chunk
        ChannelData &cd = *m_channelData[ch];
        if (!cd.draining) 
        {
            // run here every time DBG("have run here");
            size_t inbufCanReadNum = cd.inbuf->getReadSpace();//2048
            // DBG("ready min ready, m_awindowSize:" << std::min(ready, m_aWindowSize));
            assert(inbufCanReadNum >= m_aWindowSize || cd.inputSize >= 0);
            cd.inbuf->sendToFft(cd.fltbuf, std::min(inbufCanReadNum, m_aWindowSize));// last:2048
            // DBG("m_inbufJumpSampleNum:" << m_inbufJumpSampleNum);//m_inbufJumpSampleNum: 173
            cd.inbuf->skip(m_inbufJumpSampleNum);
            windowingAndFft(ch);
        }
    }
    
    bool shouldResetPhase = false;
    size_t phaseIncrement, shiftIncrement;
    if (!getIncrements(0, phaseIncrement, shiftIncrement, shouldResetPhase)) 
    {
        // DBG("have run here");//often
        calculateIncrements(phaseIncrement, shiftIncrement, shouldResetPhase);
    }

    bool last = false;
    for (size_t c = 0; c < m_channels; ++c)
    {
        last = processChunkForChannel(c, phaseIncrement, shiftIncrement, shouldResetPhase);
        m_channelData[c]->chunkSampleCount++;
    }
    // int i = 0; if (last) i = 1;
    // DBG("processOneChunk return value:" << i);
    // always return 0
    return last;
}

bool RubberBandStretcher::Impl::testInbufReadSpace(size_t c)
{
    Profiler profiler("RubberBandStretcher::Impl::testInbufReadSpace");

    ChannelData &cd = *m_channelData[c];
    RingBuffer<float> &inbuf = *cd.inbuf;

    size_t rs = inbuf.getReadSpace();

    if (rs < m_aWindowSize && !cd.draining) {
            
        if (cd.inputSize == -1) {

            // Not all the input data has been written to the inbuf
            // (that's why the input size is not yet set).  We can't
            // process, because we don't have a full chunk of data, so
            // our process chunk would contain some empty padding in
            // its input -- and that would give incorrect output, as
            // we know there is more input to come.

#ifndef NO_THREADING
            if (!m_threaded) {
#endif
                if (m_debugLevel > 1) {
                    cerr << "Note: RubberBandStretcher: read space < chunk size ("
                         << inbuf.getReadSpace() << " < " << m_aWindowSize
                         << ") when not all input written, on processChunks for channel " << c << endl;
                }

#ifndef NO_THREADING
            }
#endif
            return false;
        }
        
        if (rs == 0) {

            if (m_debugLevel > 1) {
                cerr << "read space = 0, giving up" << endl;
            }
            return false;

        } else if (rs < m_aWindowSize/2) {

            if (m_debugLevel > 1) {
                cerr << "read space = " << rs << ", setting draining true" << endl;
            }
            
            cd.draining = true;
        }
    }

    return true;
}

bool 
RubberBandStretcher::Impl::processChunkForChannel(size_t c,
                                                  size_t phaseIncrement,
                                                  size_t shiftIncrement,
                                                  bool shouldResetPhase)
{
    Profiler profiler("RubberBandStretcher::Impl::processChunkForChannel");

    // Process a single chunk on a single channel.  This assumes
    // enough input data is available; caller must have tested this
    // using e.g. testInbufReadSpace first.  
    // Return true if this is
    // the last chunk on the channel. in RT mode, the value not 1


    // DBG("processChunkForChannel: phase reset found, incrs "
    //        << phaseIncrement << ":" << shiftIncrement << "\n");
    // 173 first appear at shiftIncrement 2 times, then appear at phaseIncrement 2 times
    // most times, 2 pairs together, 256:255 happen most times

    ChannelData &cd = *m_channelData[c];

    if (!cd.draining) 
    {
        
        // This is the normal processing case -- draining is only
        // set when all the input has been used and we only need
        // to write from the existing accumulator into the output.
        
        // We know we have enough samples available in m_inbuf --
        // this is usually m_aWindowSize, but we know that if fewer
        // are available, it's OK to use zeroes for the rest
        // (which the ring buffer will provide) because we've
        // reached the true end of the data.
        
        // We need to sendToFft m_aWindowSize samples for processing, and
        // then skip m_inbufJumpSampleNum to advance the read pointer.

        modifyChunk(c, phaseIncrement, shouldResetPhase);
        synthesiseChunk(c, shiftIncrement); // reads from cd.mag, cd.phase

        //if (m_debugLevel > 2) 
        //{
        //    if (shouldResetPhase) 
        //    {
        //        for (int i = 0; i < 10; ++i) 
        //        {
        //            cd.accumulator[i] = 1.2f - (i % 3) * 1.2f;
        //        }
        //    }
        //}
    }

    bool last = false;

    // not running at all
    if (cd.draining) 
    {
        // DBG("draining: accumulator fill = " << cd.accumulatorFill << " (shiftIncrement = " << shiftIncrement << ")\n");
        if (shiftIncrement == 0) {
            // DBG("WARNING: draining: shiftIncrement == 0, can't handle that in this context: setting to " 
            //    << m_inbufJumpSampleNum << "\n");
            // shiftIncrement = m_inbufJumpSampleNum;
        }
        if (cd.accumulatorFill <= shiftIncrement) 
        {
            //DBG("reducing shift increment from " << shiftIncrement
            //              << " to " << cd.accumulatorFill
            //              << " and marking as last\n");

            shiftIncrement = cd.accumulatorFill;
            last = true;
        }
    }
        
    int required = shiftIncrement;

    if (m_outputPitchRatio != 1.0) 
    {
        //DBG(")
        required = int(required / m_outputPitchRatio) + 1;
    }

    int ws = cd.outbuf->getWriteSpace();
    if (ws < required) {
        if (m_debugLevel > 0) {
            cerr << "Buffer overrun on output for channel " << c << endl;
        }

        // The only correct thing we can do here is resize the buffer.
        // We can't wait for the client thread to read some data out
        // from the buffer so as to make more space, because the
        // client thread (if we are threaded at all) is probably stuck
        // in a process() call waiting for us to stow away enough
        // input increments to allow the process() call to complete.
        // This is an unhappy situation.

        RingBuffer<float> *oldbuf = cd.outbuf;
        cd.outbuf = oldbuf->resized(oldbuf->getSize() * 2);

        if (m_debugLevel > 1) {
            cerr << "(Write space was " << ws << ", needed " << required
                 << ": resized output buffer from " << oldbuf->getSize()
                 << " to " << cd.outbuf->getSize() << ")" << endl;
        }

        m_emergencyScavenger.claim(oldbuf);
    }

    writeChunk(c, shiftIncrement, last);
    return last;
}

void RubberBandStretcher::Impl::calculateIncrements(size_t &phaseIncrementRef,
                                               size_t &shiftIncrementRef,
                                               bool &shouldResetPhase)
{
    Profiler profiler("RubberBandStretcher::Impl::calculateIncrements");

    
    // Calculate the next upcoming phase and shift increment, on the
    // basis that both channels are in sync.  This is in contrast to
    // getIncrements, which requires that all the increments have been
    // calculated in advance but can then return increments
    // corresponding to different chunks in different channels.

    // Requires frequency domain representations of channel data in
    // the mag and phase buffers in the channel.

    // This function is only used in real-time mode.

    phaseIncrementRef = m_inbufJumpSampleNum;
    shiftIncrementRef = m_inbufJumpSampleNum;
    shouldResetPhase = false;

    if (m_channels == 0) return;

    ChannelData &cd = *m_channelData[0];

    size_t bc = cd.chunkSampleCount;
    // DBG("chunkSampleCount: " << bc);//chunkSampleCount:from 0 growing, without stopping
    for (size_t c = 1; c < m_channels; ++c) 
    {
        if (m_channelData[c]->chunkSampleCount != bc) 
        {
            cerr << "ERROR: RubberBandStretcher::Impl::calculateIncrements: Channels are not in sync" << endl;
            return;
        }
    }

    const int hs = m_fftSize/2 + 1;
    // DBG(hs);//hs:1025
    // Normally we would mix down the time-domain signal and apply a
    // single FFT, or else mix down the Cartesian form of the
    // frequency-domain signal.  Both of those would be inefficient
    // from this position.  Fortunately, the onset detectors should
    // work reasonably well (maybe even better?) if we just sum the
    // magnitudes of the frequency-domain channel signals and forget
    // about phase entirely.  Normally we don't expect the channel
    // phases to cancel each other, and broadband effects will still
    // be apparent.

    float df = 0.f;
    bool isSilent = false;

    if (m_channels == 1) 
    {
        if (sizeof(process_t) == sizeof(double)) 
        {
            df = m_phaseResetAudioCurve->processDouble((double *)cd.mag, m_inbufJumpSampleNum);
            isSilent = (m_silentAudioCurve->processDouble((double *)cd.mag, m_inbufJumpSampleNum) > 0.f);
            // DBG("df: " << df << "isSilent:\n\n"); //not run here at all
        }   
        else 
        {

            df = m_phaseResetAudioCurve->processFloat((float *)cd.mag, m_inbufJumpSampleNum);
            isSilent = (m_silentAudioCurve->processFloat((float *)cd.mag, m_inbufJumpSampleNum) > 0.f);
            // DBG("df: " << df << "isSilent:\n\n"); //not run here at all
        }
    } 
    else 
    {
        process_t *tmp = (process_t *)alloca(hs * sizeof(process_t));
        v_zero(tmp, hs);
        // hs=1025
        for (size_t c = 0; c < m_channels; ++c) 
        {
            v_add(tmp, m_channelData[c]->mag, hs);
        }

        if (sizeof(process_t) == sizeof(double)) 
        {
            df = m_phaseResetAudioCurve->processDouble((double *)tmp, m_inbufJumpSampleNum);
            isSilent = (m_silentAudioCurve->processDouble((double *)tmp, m_inbufJumpSampleNum) > 0.f);
        } 
        else 
        {
            df = m_phaseResetAudioCurve->processFloat((float *)tmp, m_inbufJumpSampleNum);
            isSilent = (m_silentAudioCurve->processFloat((float *)tmp, m_inbufJumpSampleNum) > 0.f);
        }
    }

    double frac_1_outputPitchRatio = 1.0 / m_outputPitchRatio;
    // DBG("frac_1_outputPitchRatio:" << m_outputPitchRatio);
    //frac_1_outputPitchRatio:when choose 12, 2
    //if (cd.resampler) 
    //{ 
    //    frac_1_outputPitchRatio = cd.resampler->getEffectiveRatio(frac_1_outputPitchRatio);
    //    DBG("have run here" << frac_1_outputPitchRatio); //always, change to:0.890899
    //}
    
    int incr = m_stretchCalculator->calculateSingle
        (m_outputTimeRatio, frac_1_outputPitchRatio, df, m_inbufJumpSampleNum,
         m_aWindowSize, m_sWindowSize);
    // DBG("incr:" << incr);
    //incr: most is 255/256,
    //but occasionally:-173,267,267,261,258,261,260...

    if (m_lastProcessPhaseResetDf.getWriteSpace() > 0) 
    {
        // DBG("have run here");// only at first 16 times
        m_lastProcessPhaseResetDf.write(&df, 1);
    }
    if (m_lastProcessOutputIncrements.getWriteSpace() > 0) 
    {
        // DBG("have run here"); // only at first 16 times
        m_lastProcessOutputIncrements.write(&incr, 1);
    }

    if (incr < 0) 
    {
        shouldResetPhase = true;
        incr = -incr;
    }
    // DBG("increment: " <<incr); 
    // the first few is big, but less when running 8 times
    // make -173 great again


    // The returned increment is the phase increment.  The shift
    // increment for one chunk is the same as the phase increment for
    // the following chunk (see comment below).  This means we don't
    // actually know the shift increment until we see the following
    // phase increment... which is a bit of a problem.

    // This implies we should use this increment for the shift
    // increment, and make the following phase increment the same as
    // it.  This means in RT mode we'll _____________<<be one chunk later>>>___________
    //  with our
    // phase reset than we would be in non-RT mode.  The sensitivity
    // of the broadband onset detector may mean that this isn't a
    // problem -- test it and see.

    shiftIncrementRef = incr;

    if (cd.prevIncrement == 0) 
    {
        phaseIncrementRef = shiftIncrementRef;
    } 
    else 
    {
        phaseIncrementRef = cd.prevIncrement;
    }

    cd.prevIncrement = shiftIncrementRef;

    if (isSilent) 
        ++m_silentHistory;
    else 
        m_silentHistory = 0;

    if (m_silentHistory >= int(m_aWindowSize / m_inbufJumpSampleNum) && !shouldResetPhase) 
    {
        shouldResetPhase = true;
        //DBG("calculateIncrements: phase reset on silence (isSilent history == "
        //    << m_silentHistory << ")\n");
        // at the first few times, although running, it is still mute.
        // when mute, run these block.
    }
}

bool RubberBandStretcher::Impl::getIncrements(size_t channel,
                                         size_t &phaseIncrementRef,
                                         size_t &shiftIncrementRef,
                                         bool &shouldResetPhase)
{
    Profiler profiler("RubberBandStretcher::Impl::getIncrements");

    if (channel >= m_channels) 
    {
        phaseIncrementRef = m_inbufJumpSampleNum;
        shiftIncrementRef = m_inbufJumpSampleNum;
        shouldResetPhase = false;
        return false;
    }

    // There are two relevant output increments here.  The first is
    // the phase increment which we use when recalculating the phases
    // for the current chunk; the second is the shift increment used
    // to determine how far to shift the processing buffer after
    // writing the chunk.  The shift increment for one chunk is the
    // same as the phase increment for the following chunk.
    
    // When an onset occurs for which we need to reset phases, the
    // increment given will be negative.
    
    // When we reset phases, the previous shift increment (and so
    // current phase increments) must have been m_inbufJumpSampleNum to ensure
    // consistency.
    
    // m_outputIncrements stores phase increments.

    ChannelData &cd = *m_channelData[channel];
    bool gotData = true;

    if (cd.chunkSampleCount >= m_outputIncrements.size()) {
//        cerr << "WARNING: RubberBandStretcher::Impl::getIncrements:"
//             << " chunk count " << cd.chunkSampleCount << " >= "
//             << m_outputIncrements.size() << endl;
        if (m_outputIncrements.size() == 0) {
            phaseIncrementRef = m_inbufJumpSampleNum;
            shiftIncrementRef = m_inbufJumpSampleNum;
            shouldResetPhase = false;
            return false;
        } else {
            cd.chunkSampleCount = m_outputIncrements.size()-1;
            gotData = false;
        }
    }
    
    int phaseIncrement = m_outputIncrements[cd.chunkSampleCount];
    
    int shiftIncrement = phaseIncrement;
    if (cd.chunkSampleCount + 1 < m_outputIncrements.size()) {
        shiftIncrement = m_outputIncrements[cd.chunkSampleCount + 1];
    }
    
    if (phaseIncrement < 0) {
        phaseIncrement = -phaseIncrement;
        shouldResetPhase = true;
    }
    
    if (shiftIncrement < 0) {
        shiftIncrement = -shiftIncrement;
    }
    /*
    if (shiftIncrement >= int(m_windowSize)) {
        cerr << "*** ERROR: RubberBandStretcher::Impl::processChunks: shiftIncrement " << shiftIncrement << " >= windowSize " << m_windowSize << " at " << cd.chunkSampleCount << " (of " << m_outputIncrements.size() << ")" << endl;
        shiftIncrement = m_windowSize;
    }
    */
    phaseIncrementRef = phaseIncrement;
    shiftIncrementRef = shiftIncrement;
    if (cd.chunkSampleCount == 0) shouldResetPhase = true; // don't mess with the first chunk
    return gotData;
}

void
RubberBandStretcher::Impl::windowingAndFft(size_t channel)
{
    Profiler profiler("RubberBandStretcher::Impl::windowingAndFft");

    ChannelData &cd = *m_channelData[channel];

    process_t *const R__ dblbuf = cd.dblbuf;
    float *const R__ fltbuf = cd.fltbuf;

    // cd.fltbuf is known to contain m_aWindowSize samples

    if (m_aWindowSize > m_fftSize) 
    {
        // DBG("run here");//never
        m_aSincFilter->cut(fltbuf);
    }
    //from fltbuf to dblbuf
    windowing_shiftingHalf_copyToDblbuf(dblbuf, m_fftSize, fltbuf, m_awindow);
    // DBG("have run here"); many times
    cd.fft->forwardPolar(dblbuf, cd.mag, cd.phase);
}

void RubberBandStretcher::Impl::modifyChunk(size_t channel,
                                       size_t outputIncrement,
                                       bool shouldResetPhase)
{
    Profiler profiler("RubberBandStretcher::Impl::modifyChunk");

    ChannelData &cd = *m_channelData[channel];

    //if (shouldResetPhase) 
    //{
        //DBG("phase reset: leaving phases unmodified");//often when muted and during silence, occa when playing music
    //}

    const process_t rate = process_t(m_sampleRate); 
    //DBG(rate);// change with the host
    const int count = m_fftSize / 2;
    // DBG(count);//1024
    bool unchanged = cd.unchanged && (outputIncrement == m_inbufJumpSampleNum);
    bool fullReset = shouldResetPhase;
    bool laminar = !(m_options & OptionPhaseIndependent);//banzhuangde, cengliude
    // always true

    bool bandlimited = (m_options & OptionTransientsMixed);
    // if (bandlimited)
    //    DBG("bandlimited is true");//always wrong
    int bandlow = lrint((150 * m_fftSize) / rate);
    
    int bandhigh = lrint((1000 * m_fftSize) / rate);
    // DBG("bandlow: " << bandlow << " bandhigh: " << bandhigh);
    // bandlow:7 always  bandhigh:46 always

    float freq0 = m_freq0;
    float freq1 = m_freq1;
    float freq2 = m_freq2;
    // DBG("freq0: " << freq0 << " freq1: " << freq1 << " freq2: " << freq2);
    // always:freq0: 600 freq1: 1200 freq2: 12000
    if (laminar) 
    {
        float r = getEffectiveRatio();
        // DBG("r: " << r);//effectiveRatio:0.5-2
        if (r > 1) {
            float rf0 = 600 + (600 * ((r - 1) * (r - 1) * (r - 1) * 2));
            // DBG(rf0);//600-1800 when r is 1 to 2
            float f1ratio = freq1 / freq0;//2
            
            float f2ratio = freq2 / freq0;//20
            // DBG("f1ratio: " << f1ratio << " f2ratio: " << f2ratio);//f1:2,f2:20
            freq0 = std::max(freq0, rf0);//max(600,rf0,which is more than 600 every time)

            freq1 = freq0 * f1ratio;
            freq2 = freq0 * f2ratio;
        }
    }

    int limit0 = lrint((freq0 * m_fftSize) / rate);
    int limit1 = lrint((freq1 * m_fftSize) / rate);
    int limit2 = lrint((freq2 * m_fftSize) / rate);
    // DBG(limit0 << "    " << limit1 << "    " << limit2);//change with pitch ratio
    //typical:0.5) 28 56 557
    //2) 84 167 1672
    if (limit1 < limit0) limit1 = limit0;
    if (limit2 < limit1) limit2 = limit1;
    
    process_t prevInstability = 0.0;
    bool prevDirection = false;

    process_t distance = 0.0;
    const process_t maxdist = 8.0;

    const int lookback = 1;

    process_t distacc = 0.0;

    for (int i = count; i >= 0; i -= lookback) 
    {
        // loop 1024 times
        bool resetThis = shouldResetPhase;

        if (bandlimited) 
        {
            // DBG("have run here");//never run here
            if (resetThis) 
            {
                if (i > bandlow && i < bandhigh) 
                {
                    resetThis = false;
                    fullReset = false;
                }
            }
        }

        process_t p = cd.phase[i];
        // DBG(p);//most of which range from -3 to 3, float points
        // a lot of guys before running is 0

        process_t perr = 0.0;
        process_t outphase = p;
        
        process_t mi = maxdist;
        if (i <= limit0) mi = 0.0;
        else if (i <= limit1) mi = 1.0;
        else if (i <= limit2) mi = 3.0;

        // run code inside most times
        if (!resetThis) 
        {
            process_t omega = (2 * M_PI * m_inbufJumpSampleNum * i) / (m_fftSize);

            process_t pp = cd.prevPhase[i];

            process_t ep = pp + omega;
            perr = princarg(p - ep);
            // DBG(perr);//perr between -3.14 and 3.14
            process_t instability = fabs(perr - cd.prevError[i]);
            // DBG(instability);// usually under 0.1, but sometimes up to 3
            bool direction = (perr > cd.prevError[i]);
            // if (!direction) DBG("direction is set to 0");  
            //both possibilities is often happens
            bool inherit = false;

            if (laminar) 
            {
                int i = 0;
                // DBG("mi:" << mi); // int ,change with pitch ratio, if pitch ratio is more than 0,
                // it would be 8 or 3
                if (distance >= mi || i == count) 
                {
                    
                    // DBG(distance); //sometimes 0, sometimes 1
                    inherit = false;
                } 
                else if (bandlimited && (i == bandhigh || i == bandlow)) 
                {
                    // DBG("never run here");// never run here
                    inherit = false;
                } 
                else if (instability > prevInstability && direction == prevDirection) 
                {
            
                    // DBG("inherit is true"); // often run here
                    inherit = true;
                }
            }
            // else { DBG("no laminer"); }// never run here

            // DBG(m_inbufJumpSampleNum);// change with ratio, 455 when lower, 157 when higher, eg.
            process_t advance = outputIncrement * ((omega + perr) / m_inbufJumpSampleNum);
            //DBG("outputIncr: " << outputIncrement << "   omega: " << omega << " perr " << perr << " m_incr: " << advance);
            // outputIncr sometimes fall to 157 or so ,most time 268/265 / 269 by a group,sometimes down to 70/80/69 as a group
            // omega: up to 600, then down to 0 almost step by step(hop size 1)
            // perr: always low, like -0.9, -2.19, -2.85, 2.50...change without discipline
            // m_incr floatpoint, down to 140/135/120, but can up to 809
            // 
            // DBG("advance: " << advance);
            // advance range from 0 to 841.947, and downgrade almost one by one back to 0

            if (inherit) 
            {
                process_t inherited = cd.unwrappedPhase[i + lookback] - cd.prevPhase[i + lookback];
                // DBG(inherited);// up to 20000+, down to 0, in all low down, but not always, sometimes suddenly downup
                advance = ((advance * distance) + (inherited * (maxdist - distance))) / maxdist;
                // DBG(advance); // can up to 17000 eg. not always low, sometimes higher than others

                outphase = p + advance;
                // DBG(outphase);//usually thousands, hundreds, up to 9000+, down to 0
                distacc += distance;
                distance += 1.0;
            } 
            else
            {
                outphase = cd.unwrappedPhase[i] + advance;
                distance = 0.0;
            }

            prevInstability = instability;
            // DBG(prevInstability); may be 3.5, usually small
            prevDirection = direction;
        } 
        else 
        {
            distance = 0.0;
        }

        cd.prevError[i] = perr;
        
        cd.prevPhase[i] = p;
        cd.phase[i] = outphase;
        cd.unwrappedPhase[i] = outphase;
    }


    // DBG("mean inheritance distance = " << distacc / count <<"\n");
    //usually between 0.009 and 0.035

    if (fullReset) unchanged = true;
    cd.unchanged = unchanged;

    // if (unchanged) DBG("frame unchanged on channel " << channel << "\n");// jiaocuo, (staggered, occa)
    // sync to resetThis
}    


void RubberBandStretcher::Impl::formantShiftChunk(size_t channel)
{
    Profiler profiler("RubberBandStretcher::Impl::formantShiftChunk");

    ChannelData &cd = *m_channelData[channel];

    process_t *const R__ mag = cd.mag;
    process_t *const R__ envelope = cd.envelope;
    process_t *const R__ dblbuf = cd.dblbuf;

    const int sz = m_fftSize;
    const int hs = sz / 2;
    const process_t factor = 1.0 / sz;

    cd.fft->inverseCepstral(mag, dblbuf);

    const int cutoff = m_sampleRate / 700;

//    cerr <<"cutoff = "<< cutoff << ", m_sampleRate/cutoff = " << m_sampleRate/cutoff << endl;

    dblbuf[0] /= 2;
    dblbuf[cutoff-1] /= 2;

    for (int i = cutoff; i < sz; ++i) 
    {
        dblbuf[i] = 0.0;
    }

    v_scale(dblbuf, factor, cutoff);

    process_t *spare = (process_t *)alloca((hs + 1) * sizeof(process_t));
    cd.fft->forward(dblbuf, envelope, spare);

    v_exp(envelope, hs + 1);
    v_divide(mag, envelope, hs + 1);

    if (m_outputPitchRatio > 1.0) {
        // scaling up, we want a new envelope that is lower by the pitch factor
        for (int target = 0; target <= hs; ++target) {
            int source = lrint(target * m_outputPitchRatio);
            if (source > hs) {
                envelope[target] = 0.0;
            } else {
                envelope[target] = envelope[source];
            }
        }
    } else {
        // scaling down, we want a new envelope that is higher by the pitch factor
        for (int target = hs; target > 0; ) {
            --target;
            int source = lrint(target * m_outputPitchRatio);
            envelope[target] = envelope[source];
        }
    }

    v_multiply(mag, envelope, hs+1);

    cd.unchanged = false;
}

void
RubberBandStretcher::Impl::synthesiseChunk(size_t channel,
                                           size_t shiftIncrement)
{
    Profiler profiler("RubberBandStretcher::Impl::synthesiseChunk");


    if ((m_options & OptionFormantPreserved) &&
        (m_outputPitchRatio != 1.0)) 
    {
        // DBG("have run here"); // never run here
        formantShiftChunk(channel);
    }

    ChannelData &cd = *m_channelData[channel];

    process_t *const R__ dblbuf = cd.dblbuf;
    float *const R__ fltbuf = cd.fltbuf;
    float *const R__ accumulator = cd.accumulator;
    float *const R__ windowAccumulator = cd.windowAccumulator;
    
    const int fsz = m_fftSize;
    const int hs = fsz / 2;

    const int wsz = m_sWindowSize;

    if (!cd.unchanged) 
    {
        // DBG("cd.unchanged is right"); // most times run here
        // Our FFTs produced unscaled results. Scale before inverse
        // transform rather than after, to avoid overflow if using a
        // fixed-point FFT.
        float factor = 1.f / fsz;
        v_scale(cd.mag, factor, hs + 1);

        cd.fft->inversePolar(cd.mag, cd.phase, cd.dblbuf);

        if (wsz == fsz) 
        {
            v_convert(fltbuf, dblbuf + hs, hs);
            v_convert(fltbuf + hs, dblbuf, hs);
        } 
        else 
        {
            v_zero(fltbuf, wsz);
            int j = fsz - wsz/2;
            while (j < 0) j += fsz;
            for (int i = 0; i < wsz; ++i) 
            {
                fltbuf[i] += dblbuf[j];
                if (++j == fsz) j = 0;
            }
        }
    }

    if (wsz > fsz) 
    {
        // DBG("have run here"); //never run here
        int p = shiftIncrement * 2;
        if (cd.interpolatorScale != p) 
        {
            SincWindow<float>::write(cd.interpolator, wsz, p);
            cd.interpolatorScale = p;
        }
        v_multiply(fltbuf, cd.interpolator, wsz);
    }

    m_swindow->cut(fltbuf);
    v_add(accumulator, fltbuf, wsz);
    cd.accumulatorFill = std::max(cd.accumulatorFill, size_t(wsz));

    if (wsz > fsz) 
    {
        // reuse fltbuf to calculate interpolating window shape for
        // window accumulator
        // DBG("have run here");// never run here
        v_copy(fltbuf, cd.interpolator, wsz);
        m_swindow->cut(fltbuf);
        v_add(windowAccumulator, fltbuf, wsz);
    } 
    else 
    {
        m_swindow->add(windowAccumulator, m_awindow->getArea() * 1.5f);
    }
}

void RubberBandStretcher::Impl::writeChunk(size_t channel, size_t shiftIncrement, bool last)
{
    Profiler profiler("RubberBandStretcher::Impl::writeChunk");

    ChannelData &cd = *m_channelData[channel];
    
    float *const R__ accumulator = cd.accumulator;
    float *const R__ windowAccumulator = cd.windowAccumulator;

    const int sz = cd.accumulatorFill;
    const int si = shiftIncrement;

    
    // DBG("writeChunk(" << channel << ", " << shiftIncrement << ", "  << ")\n");
    // writeChunk(0/1, 255/256/269,sometimes down to 173 or so
    // 173 appears as a group
    

    v_divide(accumulator, windowAccumulator, si);

    // for exact sample scaling (probably not meaningful if we
    // were running in RT mode)
    size_t theoreticalOut = 0;
    if (cd.inputSize >= 0) {
        theoreticalOut = lrint(cd.inputSize * m_outputTimeRatio);
    }

    bool resampledAlready = resampleBeforeStretching();

    if (!resampledAlready &&
        (m_outputPitchRatio != 1.0 || m_options & OptionPitchHighConsistency) &&
        cd.resampler) 
    {

        Profiler profiler2("RubberBandStretcher::Impl::resample");

        size_t reqSize = int(ceil(si / m_outputPitchRatio));
        if (reqSize > cd.resamplebufSize) {
            // This shouldn't normally happen -- the buffer is
            // supposed to be initialised with enough space in the
            // first place.  But we retain this check in case the
            // pitch scale has changed since then, or the stretch
            // calculator has gone mad, or something.
            
            // never happens
            // DBG( "WARNING: RubberBandStretcher::Impl::writeChunk: resizing resampler buffer from "
            //           << cd.resamplebufSize << " to " << reqSize <<"\n");
            cd.setResampleBufSize(reqSize);
        }

#ifndef NO_THREADING
#if defined HAVE_IPP && !defined USE_SPEEX
        if (m_threaded) {
            m_resamplerMutex.lock();
        }
#endif
#endif

        size_t outframes = cd.resampler->resample(&cd.resamplebuf,
                                                  cd.resamplebufSize,
                                                  &cd.accumulator,
                                                  si,
                                                  1.0 / m_outputPitchRatio,
                                                  last);

#ifndef NO_THREADING
#if defined HAVE_IPP && !defined USE_SPEEX
        if (m_threaded) {
            m_resamplerMutex.unlock();
        }
#endif
#endif

        writeOutput(*cd.outbuf, cd.resamplebuf,
                    outframes, cd.outCount, theoreticalOut);

    } else {
        writeOutput(*cd.outbuf, accumulator,
                    si, cd.outCount, theoreticalOut);
    }

    v_move(accumulator, accumulator + si, sz - si);
    v_zero(accumulator + sz - si, si);
    
    v_move(windowAccumulator, windowAccumulator + si, sz - si);
    v_zero(windowAccumulator + sz - si, si);
    
    if (int(cd.accumulatorFill) > si) {
        cd.accumulatorFill -= si;
    } else {
        cd.accumulatorFill = 0;
        if (cd.draining) {
            if (m_debugLevel > 1) {
                cerr << "RubberBandStretcher::Impl::processChunks: setting outputComplete to true" << endl;
            }
            cd.outputComplete = true;
        }
    }
}

void
RubberBandStretcher::Impl::writeOutput(RingBuffer<float> &to, float *from, size_t qty, size_t &outCount, size_t theoreticalOut)
{
    Profiler profiler("RubberBandStretcher::Impl::writeOutput");

    // In non-RT mode, we don't want to write the first startSkip
    // samples, because the first chunk is centred on the start of the
    // output.  In RT mode we didn't apply any pre-padding in
    // configure(), so we don't want to remove any here.

    size_t startSkip = 0;
    if (!m_realtime) {
        startSkip = lrintf((m_sWindowSize/2) / m_outputPitchRatio);
    }

    if (outCount > startSkip) {
        
        // this is the normal case

        if (theoreticalOut > 0) {
            if (m_debugLevel > 1) {
                cerr << "theoreticalOut = " << theoreticalOut
                     << ", outCount = " << outCount
                     << ", startSkip = " << startSkip
                     << ", qty = " << qty << endl;
            }
            if (outCount - startSkip <= theoreticalOut &&
                outCount - startSkip + qty > theoreticalOut) {
                qty = theoreticalOut - (outCount - startSkip);
                if (m_debugLevel > 1) {
                    cerr << "reduce qty to " << qty << endl;
                }
            }
        }

        if (m_debugLevel > 2) {
            cerr << "writing " << qty << endl;
        }

        size_t written = to.write(from, qty);

        if (written < qty) {
            cerr << "WARNING: RubberBandStretcher::Impl::writeOutput: "
                 << "Buffer overrun on output: wrote " << written
                 << " of " << qty << " samples" << endl;
        }

        outCount += written;
        return;
    }

    // the rest of this is only used during the first startSkip samples

    if (outCount + qty <= startSkip) {
        if (m_debugLevel > 1) {
            cerr << "qty = " << qty << ", startSkip = "
                 << startSkip << ", outCount = " << outCount
                 << ", discarding" << endl;
        }
        outCount += qty;
        return;
    }

    size_t off = startSkip - outCount;
    if (m_debugLevel > 1) {
        cerr << "qty = " << qty << ", startSkip = "
             << startSkip << ", outCount = " << outCount
             << ", writing " << qty - off
             << " from start offset " << off << endl;
    }
    to.write(from + off, qty - off);
    outCount += qty;
}

int
RubberBandStretcher::Impl::available() const
{
    Profiler profiler("RubberBandStretcher::Impl::available");

#ifndef NO_THREADING
    if (m_threaded) {
        MutexLocker locker(&m_threadSetMutex);
        if (m_channelData.empty()) return 0;
    } else {
        if (m_channelData.empty()) return 0;
    }
#endif

#ifndef NO_THREADING
    if (!m_threaded) 
    {
#endif
        for (size_t c = 0; c < m_channels; ++c) 
        {
            if (m_channelData[c]->inputSize >= 0) 
            {
//                cerr << "available: m_done true" << endl;
                if (m_channelData[c]->inbuf->getReadSpace() > 0) 
                {
                    // DBG("calling processChunks(" << c << ") from available\n");
                    //!!! do we ever actually do this? if so, this method should not be const
                    // ^^^ yes, we do sometimes -- e.g. when fed a very short file
                    bool any = false, last = false;
                    ((RubberBandStretcher::Impl *)this)->processChunks(c, any, last);
                }
            }
        }
#ifndef NO_THREADING
    }
#endif

    size_t min = 0;
    bool consumed = true;
    bool haveResamplers = false;

    for (size_t i = 0; i < m_channels; ++i) 
    {
        size_t availIn = m_channelData[i]->inbuf->getReadSpace();
        size_t availOut = m_channelData[i]->outbuf->getReadSpace();


        // DBG("available on channel " << i << ": " << availOut << " (waiting: " << availIn << ")\n");


        if (i == 0 || availOut < min) 
            min = availOut;
        if (!m_channelData[i]->outputComplete) 
            consumed = false;
        if (m_channelData[i]->resampler) 
            haveResamplers = true;
    }

    if (min == 0 && consumed) 
        return -1;
    if (m_outputPitchRatio == 1.0) 
        return min;

    if (haveResamplers) 
        return min; // shouldResampleBeforeStretching has already happened

    return int(floor(min / m_outputPitchRatio));
}

size_t RubberBandStretcher::Impl::retrieve(float *const *output, size_t samples) const
{
    Profiler profiler("RubberBandStretcher::Impl::retrieve");

    size_t got = samples;

    for (size_t c = 0; c < m_channels; ++c) 
    {
        size_t gotHere = m_channelData[c]->outbuf->read(output[c], got);
        if (gotHere < got) 
        {
            if (c > 0) DBG("RubberBandStretcher::Impl::retrieve: WARNING: channel imbalance detected\n");
            
            got = gotHere;
        }
    }

    if ((m_options & OptionChannelsTogether) && (m_channels >= 2)) 
    {
        // DBG("what"); //never go here
        for (size_t i = 0; i < got; ++i) 
        {
            float mid = output[0][i];
            float side = output[1][i];
            float left = mid + side;
            float right = mid - side;
            output[0][i] = left;
            output[1][i] = right;
        }
    }            
    // DBG(got); // different every time called
    return got;
}

}

