
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#define FLOAT_SAMPLES
#define SUPPORT_SSE         0x0008
typedef unsigned long long ulongptr;
// #include "STTypes.h"
// #include "SoundTouch/cpu_detect.h"
#include "MatchBuffer.h"
#define SOUNDTOUCH_ALIGN_POINTER_16(x)      ( ( (ulongptr)(x) + 15 ) & ~(ulongptr)15 )

#define max(x, y) (((x) > (y)) ? (x) : (y))

/*****************************************************************************
 *
 * Constant definitions
 *
 *****************************************************************************/

 // Table for the hierarchical mixing position seeking algorithm
const short _scanOffsets[5][24] = {
    { 124,  186,  248,  310,  372,  434,  496,  558,  620,  682,  744, 806,
      868,  930,  992, 1054, 1116, 1178, 1240, 1302, 1364, 1426, 1488,   0},
    {-100,  -75,  -50,  -25,   25,   50,   75,  100,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
    { -20,  -15,  -10,   -5,    5,   10,   15,   20,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
    {  -4,   -3,   -2,   -1,    1,    2,    3,    4,    0,    0,    0,   0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
    { 121,  114,   97,  114,   98,  105,  108,   32,  104,   99,  117,  111,
      116,  100,  110,  117,  111,  115,    0,    0,    0,    0,    0,   0} };

/*****************************************************************************
 *
 * Implementation of the class 'BufferMatch'
 *
 *****************************************************************************/


BufferMatch::BufferMatch() : FIFOProcessor(&outputBuffer)
{
    bQuickSeek = false;
    channels = 2;

    pMidBuffer = NULL;
    pMidBufferUnaligned = NULL;
    overlapLength = 0;

    bAutoSeqSetting = true;
    bAutoSeekSetting = true;

    tempo = 1.0f;
    setParameters(44100, DEFAULT_SEQUENCE_MS, DEFAULT_SEEKWINDOW_MS, DEFAULT_OVERLAP_MS);
    setTempo(1.0f);

    clear();
}



BufferMatch::~BufferMatch()
{
    delete[] pMidBufferUnaligned;
}



// Sets routine control parameters. These control are certain time constants
// defining how the sound is stretched to the desired duration.
//
// 'sampleRate' = sample rate of the sound
// 'sequenceMS' = one processing sequence length in milliseconds (default = 82 ms)
// 'seekwindowMS' = seeking window length for scanning the best overlapping 
//      position (default = 28 ms)
// 'overlapMS' = overlapping length (default = 12 ms)

void BufferMatch::setParameters(int aSampleRate, int aSequenceMS,
    int aSeekWindowMS, int aOverlapMS)
{
    // accept only positive parameter values - if zero or negative, use old values instead
    if (aSampleRate > 0)
    {
        if (aSampleRate > 192000) THROW_RT_ERROR("Error: Excessive samplerate");
        this->sampleRate = aSampleRate;
    }

    if (aOverlapMS > 0) this->overlapMs = aOverlapMS;

    if (aSequenceMS > 0)
    {
        this->sequenceMs = aSequenceMS;
        bAutoSeqSetting = false;
    }
    else if (aSequenceMS == 0)
    {
        // if zero, use automatic setting
        bAutoSeqSetting = true;
    }

    if (aSeekWindowMS > 0)
    {
        this->seekWindowMs = aSeekWindowMS;
        bAutoSeekSetting = false;
    }
    else if (aSeekWindowMS == 0)
    {
        // if zero, use automatic setting
        bAutoSeekSetting = true;
    }

    calcSeqParameters();

    calculateOverlapLength(overlapMs);

    // set tempo to recalculate 'sampleReq'
    setTempo(tempo);
}



/// Get routine control parameters, see setParameters() function.
/// Any of the parameters to this function can be NULL, in such case corresponding parameter
/// value isn't returned.
void BufferMatch::getParameters(int* pSampleRate, int* pSequenceMs, int* pSeekWindowMs, int* pOverlapMs) const
{
    if (pSampleRate)
    {
        *pSampleRate = sampleRate;
    }

    if (pSequenceMs)
    {
        *pSequenceMs = (bAutoSeqSetting) ? (USE_AUTO_SEQUENCE_LEN) : sequenceMs;
    }

    if (pSeekWindowMs)
    {
        *pSeekWindowMs = (bAutoSeekSetting) ? (USE_AUTO_SEEKWINDOW_LEN) : seekWindowMs;
    }

    if (pOverlapMs)
    {
        *pOverlapMs = overlapMs;
    }
}


// Overlaps samples in 'midBuffer' with the samples in 'pInput'
void BufferMatch::overlapMono(SAMPLETYPE* pOutput, const SAMPLETYPE* pInput) const
{
    int i;
    SAMPLETYPE m1, m2;

    m1 = (SAMPLETYPE)0;
    m2 = (SAMPLETYPE)overlapLength;

    for (i = 0; i < overlapLength; i++)
    {
        pOutput[i] = (pInput[i] * m1 + pMidBuffer[i] * m2) / overlapLength;
        m1 += 1;
        m2 -= 1;
    }
}



void BufferMatch::clearMidBuffer()
{
    memset(pMidBuffer, 0, channels * sizeof(SAMPLETYPE) * overlapLength);
}


void BufferMatch::clearInput()
{
    inputBuffer.clear();
    clearMidBuffer();
    isBeginning = true;
    maxnorm = 0;
    maxnormf = 1e8;
    skipFract = 0;
}


// Clears the sample buffers
void BufferMatch::clear()
{
    outputBuffer.clear();
    clearInput();
}



// Enables/disables the quick position seeking algorithm. Zero to disable, nonzero
// to enable
void BufferMatch::enableQuickSeek(bool enable)
{
    bQuickSeek = enable;
}


// Returns nonzero if the quick seeking algorithm is enabled.
bool BufferMatch::isQuickSeekEnabled() const
{
    return bQuickSeek;
}


// Seeks for the optimal overlap-mixing position.
int BufferMatch::seekBestOverlapPosition(const SAMPLETYPE* refPos)
{
    if (bQuickSeek)
    {
        return seekBestOverlapPositionQuick(refPos);
    }
    else
    {

        return seekBestOverlapPositionFull(refPos);
    }
}


// Overlaps samples in 'midBuffer' with the samples in 'pInputBuffer' at position
// of 'ovlPos'.
inline void BufferMatch::overlap(SAMPLETYPE* pOutput, const SAMPLETYPE* pInput, uint ovlPos) const
{
    if (channels == 1)
    {
        // mono sound.
        overlapMono(pOutput, pInput + ovlPos);
    }
}


// Seeks for the optimal overlap-mixing position. The 'stereo' version of the
// routine
//
// The best position is determined as the position where the two overlapped
// sample sequences are 'most alike', in terms of the highest cross-correlation
// value over the overlapping period
int BufferMatch::seekBestOverlapPositionFull(const SAMPLETYPE* refPos)
{
    int bestOffs;
    double bestCorr;
    int i;
    double norm;

    bestCorr = -FLT_MAX;
    bestOffs = 0;

    // Scans for the best correlation value by testing each possible position
    // over the permitted range.
    bestCorr = calcCrossCorr(refPos, pMidBuffer, norm);
    bestCorr = (bestCorr + 0.1) * 0.75;
    // DBG(bestCorr); // range from -1.4 to 2 or so, in float mode
    // DBG(seekLength); // 1102
#pragma omp parallel for
    for (i = 1; i < seekLength; i++)
    {
        double corr;
        // Calculates correlation value for the mixing position corresponding to 'i'
#if defined(_OPENMP) || defined(ST_SIMD_AVOID_UNALIGNED)
        // in parallel OpenMP mode, can't use norm accumulator version as parallel executor won't
        // iterate the loop in sequential order
        // in SIMD mode, avoid accumulator version to allow avoiding unaligned positions
        corr = calcCrossCorr(refPos + channels * i, pMidBuffer, norm);
#else
        // In non-parallel version call "calcCrossCorrAccumulate" that is otherwise same
        // as "calcCrossCorr", but saves time by reusing & updating previously stored 
        // "norm" value
        corr = calcCrossCorrAccumulate(refPos + channels * i, pMidBuffer, norm);
#endif
        // heuristic rule to slightly favour values close to mid of the range
        double tmp = (double)(2 * i - seekLength) / (double)seekLength;
        // seekLength: 1102
        corr = ((corr + 0.1) * (1.0 - 0.25 * tmp * tmp));

        // Checks for the highest correlation value
        if (corr > bestCorr)
        {
            // For optimal performance, enter critical section only in case that best value found.
            // in such case repeat 'if' condition as it's possible that parallel execution may have
            // updated the bestCorr value in the mean time
#pragma omp critical
            if (corr > bestCorr)
            {
                // DBG(bestOffs);// often 277, 609, 399, 480, 685, 896 or so?
                bestCorr = corr;
                bestOffs = i;
            }
        }
    }

#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    adaptNormalizer();
#endif

    // clear cross correlation routine state if necessary (is so e.g. in MMX routines).
    clearCrossCorrState();

    return bestOffs;
}


// Quick seek algorithm for improved runtime-performance: First roughly scans through the 
// correlation area, and then scan surroundings of two best preliminary correlation candidates
// with improved precision
//
// Based on testing:
// - This algorithm gives on average 99% as good match as the full algorithm
// - this quick seek algorithm finds the best match on ~90% of cases
// - on those 10% of cases when this algorithm doesn't find best match, 
//   it still finds on average ~90% match vs. the best possible match
int BufferMatch::seekBestOverlapPositionQuick(const SAMPLETYPE* refPos)
{
#define _MIN(a, b)   (((a) < (b)) ? (a) : (b))
#define SCANSTEP    16
#define SCANWIND    8

    int bestOffs;
    int i;
    int bestOffs2;
    float bestCorr, corr;
    float bestCorr2;
    double norm;

    // note: 'float' types used in this function in case that the platform would need to use software-fp

    bestCorr =
        bestCorr2 = -FLT_MAX;
    bestOffs =
        bestOffs2 = SCANWIND;

    // Scans for the best correlation value by testing each possible position
    // over the permitted range. Look for two best matches on the first pass to
    // increase possibility of ideal match.
    //
    // Begin from "SCANSTEP" instead of SCANWIND to make the calculation
    // catch the 'middlepoint' of seekLength vector as that's the a-priori 
    // expected best match position
    //
    // Roughly:
    // - 15% of cases find best result directly on the first round,
    // - 75% cases find better match on 2nd round around the best match from 1st round
    // - 10% cases find better match on 2nd round around the 2nd-best-match from 1st round
    for (i = SCANSTEP; i < seekLength - SCANWIND - 1; i += SCANSTEP)
    {
        // Calculates correlation value for the mixing position corresponding
        // to 'i'
        corr = (float)calcCrossCorr(refPos + channels * i, pMidBuffer, norm);
        // heuristic rule to slightly favour values close to mid of the seek range
        float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
        corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

        // Checks for the highest correlation value
        if (corr > bestCorr)
        {
            // found new best match. keep the previous best as 2nd best match
            bestCorr2 = bestCorr;
            bestOffs2 = bestOffs;
            bestCorr = corr;
            bestOffs = i;
        }
        else if (corr > bestCorr2)
        {
            // not new best, but still new 2nd best match
            bestCorr2 = corr;
            bestOffs2 = i;
        }
    }

    // Scans surroundings of the found best match with small stepping
    int end = _MIN(bestOffs + SCANWIND + 1, seekLength);
    for (i = bestOffs - SCANWIND; i < end; i++)
    {
        if (i == bestOffs) continue;    // this offset already calculated, thus skip

        // Calculates correlation value for the mixing position corresponding
        // to 'i'
        corr = (float)calcCrossCorr(refPos + channels * i, pMidBuffer, norm);
        // heuristic rule to slightly favour values close to mid of the range
        float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
        corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

        // Checks for the highest correlation value
        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestOffs = i;
        }
    }

    // Scans surroundings of the 2nd best match with small stepping
    end = _MIN(bestOffs2 + SCANWIND + 1, seekLength);
    for (i = bestOffs2 - SCANWIND; i < end; i++)
    {
        if (i == bestOffs2) continue;    // this offset already calculated, thus skip

        // Calculates correlation value for the mixing position corresponding
        // to 'i'
        corr = (float)calcCrossCorr(refPos + channels * i, pMidBuffer, norm);
        // heuristic rule to slightly favour values close to mid of the range
        float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
        corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

        // Checks for the highest correlation value
        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestOffs = i;
        }
    }

    // clear cross correlation routine state if necessary (is so e.g. in MMX routines).
    clearCrossCorrState();

#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    adaptNormalizer();
#endif

    return bestOffs;
}




/// For integer algorithm: adapt normalization factor divider with music so that 
/// it'll not be pessimistically restrictive that can degrade quality on quieter sections
/// yet won't cause integer overflows either
void BufferMatch::adaptNormalizer()
{
    // Do not adapt normalizer over too silent sequences to avoid averaging filter depleting to
    // too low values during pauses in music
    if ((maxnorm > 1000) || (maxnormf > 40000000))
    {
        //norm averaging filter
        maxnormf = 0.9f * maxnormf + 0.1f * (float)maxnorm;

        if ((maxnorm > 800000000) && (overlapDividerBitsNorm < 16))
        {
            // large values, so increase divider
            overlapDividerBitsNorm++;
            if (maxnorm > 1600000000) overlapDividerBitsNorm++; // extra large value => extra increase
        }
        else if ((maxnormf < 1000000) && (overlapDividerBitsNorm > 0))
        {
            // extra small values, decrease divider
            overlapDividerBitsNorm--;
        }
    }

    maxnorm = 0;
}


/// clear cross correlation routine state if necessary 
void BufferMatch::clearCrossCorrState()
{
    // default implementation is empty.
}


/// Calculates processing sequence length according to tempo setting
void BufferMatch::calcSeqParameters()
{
    // Adjust tempo param according to tempo, so that variating processing sequence length is used
    // at various tempo settings, between the given low...top limits
#define AUTOSEQ_TEMPO_LOW   0.5     // auto setting low tempo range (-50%)
#define AUTOSEQ_TEMPO_TOP   2.0     // auto setting top tempo range (+100%)

// sequence-ms setting values at above low & top tempo
#define AUTOSEQ_AT_MIN      90.0
#define AUTOSEQ_AT_MAX      40.0
#define AUTOSEQ_K           ((AUTOSEQ_AT_MAX - AUTOSEQ_AT_MIN) / (AUTOSEQ_TEMPO_TOP - AUTOSEQ_TEMPO_LOW))
#define AUTOSEQ_C           (AUTOSEQ_AT_MIN - (AUTOSEQ_K) * (AUTOSEQ_TEMPO_LOW))

// seek-window-ms setting values at above low & top tempoq
#define AUTOSEEK_AT_MIN     20.0
#define AUTOSEEK_AT_MAX     15.0
#define AUTOSEEK_K          ((AUTOSEEK_AT_MAX - AUTOSEEK_AT_MIN) / (AUTOSEQ_TEMPO_TOP - AUTOSEQ_TEMPO_LOW))
#define AUTOSEEK_C          (AUTOSEEK_AT_MIN - (AUTOSEEK_K) * (AUTOSEQ_TEMPO_LOW))

#define CHECK_LIMITS(x, mi, ma) (((x) < (mi)) ? (mi) : (((x) > (ma)) ? (ma) : (x)))

    double seq, seek;

    if (bAutoSeqSetting)
    {
        // DBG("have run here");
        seq = AUTOSEQ_C + AUTOSEQ_K * tempo;
        seq = CHECK_LIMITS(seq, AUTOSEQ_AT_MAX, AUTOSEQ_AT_MIN);
        sequenceMs = (int)(seq + 0.5);
    }

    if (bAutoSeekSetting)
    {
        // DBG("have run here");
        seek = AUTOSEEK_C + AUTOSEEK_K * tempo;
        seek = CHECK_LIMITS(seek, AUTOSEEK_AT_MAX, AUTOSEEK_AT_MIN);
        seekWindowMs = (int)(seek + 0.5);
    }

    // Update seek window lengths
    seekWindowLength = (sampleRate * sequenceMs) / 1000;
    if (seekWindowLength < 2 * overlapLength)
    {
        seekWindowLength = 2 * overlapLength;
    }
    seekLength = (sampleRate * seekWindowMs) / 1000;
}



// Sets new target tempo. Normal tempo = 'SCALE', smaller values represent slower 
// tempo, larger faster tempo.
void BufferMatch::setTempo(double newTempo)
{
    int intskip;

    tempo = newTempo;

    // Calculate new sequence duration
    calcSeqParameters();
    // DBG(newTempo);
    // Calculate ideal skip length (according to tempo value) 
    nominalSkip = tempo * (seekWindowLength - overlapLength);
    intskip = (int)(nominalSkip + 0.5);

    // Calculate how many samples are needed in the 'inputBuffer' to 
    // process another batch of samples
    //sampleReq = max(intskip + overlapLength, seekWindowLength) + seekLength / 2;


    //DBG("intskip: " << intskip << " overlapLength: " << overlapLength << " seekWindowLength: " << seekWindowLength
    //    << " seekLength: " << seekLength);
    // seekWindowLength: 2646, seekLength: 1102,  intskip: range from 1147(max) to 4588(min)
    // overlapLength: 352

    //48000: 4992~1248, 384, 2880 1200

    // at the beginning, seekLength 793~864~1200 to steady
    // seekWindowLength 3219 3504 2880 to steady
    // intskip 2867 3120 2496 3600 1800 1248 to stead
    sampleReq = max(intskip + overlapLength, seekWindowLength) + seekLength;
}



// Sets the number of channels, 1 = mono, 2 = stereo
void BufferMatch::setChannels(int numChannels)
{
    if (!verifyNumberOfChannels(numChannels) ||
        (channels == numChannels)) return;

    channels = numChannels;
    inputBuffer.setChannels(channels);
    outputBuffer.setChannels(channels);

    // re-init overlap/buffer
    overlapLength = 0;
    setParameters(sampleRate);
}


// nominal tempo, no need for processing, just pass the samples through
// to outputBuffer
/*
void BufferMatch::processNominalTempo()
{
    assert(tempo == 1.0f);

    if (bMidBufferDirty)
    {
        // If there are samples in pMidBuffer waiting for overlapping,
        // do a single sliding overlapping with them in order to prevent a
        // clicking distortion in the output sound
        if (inputBuffer.numSamples() < overlapLength)
        {
            // wait until we've got overlapLength input samples
            return;
        }
        // Mix the samples in the beginning of 'inputBuffer' with the
        // samples in 'midBuffer' using sliding overlapping
        overlap(outputBuffer.ptrEnd(overlapLength), inputBuffer.ptrBegin(), 0);
        outputBuffer.putSamples(overlapLength);
        inputBuffer.receiveSamples(overlapLength);
        clearMidBuffer();
        // now we've caught the nominal sample flow and may switch to
        // bypass mode
    }

    // Simply bypass samples from input to output
    outputBuffer.moveSamples(inputBuffer);
}
*/


// Processes as many processing frames of the samples 'inputBuffer', store
// the result into 'outputBuffer'
void BufferMatch::processSamples()
{
    int ovlSkip;
    int offset = 0;
    int temp;

    /* Removed this small optimization - can introduce a click to sound when tempo setting
       crosses the nominal value
    if (tempo == 1.0f)
    {
        // tempo not changed from the original, so bypass the processing
        processNominalTempo();
        return;
    }
    */

    // Process samples as long as there are enough samples in 'inputBuffer'
    // to form a processing frame.

    while ((int)inputBuffer.numSamples() >= sampleReq)
    {


        if (isBeginning == false)
        {
            // apart from the very beginning of the track, 
            // scan for the best overlapping position & do overlap-add
            offset = seekBestOverlapPosition(inputBuffer.ptrBegin());
            // DBG(offset);
            //224~1070


            // Mix the samples in the 'inputBuffer' at position of 'offset' with the 
            // samples in 'midBuffer' using sliding overlapping
            // ... first partially overlap with the end of the previous sequence
            // (that's in 'midBuffer')
            overlap(outputBuffer.ptrEnd((uint)overlapLength), inputBuffer.ptrBegin(), (uint)offset);
            outputBuffer.putSamples((uint)overlapLength);
            offset += overlapLength;
        }
        else
        {
            // Adjust processing offset at beginning of track by not perform initial overlapping
            // and compensating that in the 'input buffer skip' calculation

            // only runs when beginning and change samplesPerBlock or sampleRate, when every object is constructed again
            isBeginning = false;

            int skip = (int)(tempo * overlapLength + 0.5 * seekLength + 0.5);
#ifdef ST_SIMD_AVOID_UNALIGNED
// in SIMD mode, round the skip amount to value corresponding to aligned memory address
            if (channels == 1)
            {
                skip &= -4;
            }
            else if (channels == 2)
            {
                skip &= -2;
            }
#endif
            skipFract -= skip;


            if (skipFract <= -nominalSkip)
            {
                // never run here
                skipFract = -nominalSkip;
            }
        }

        // ... then copy sequence samples from 'inputBuffer' to output:

        // crosscheck that we don't have buffer overflow...
        if ((int)inputBuffer.numSamples() < (offset + seekWindowLength - overlapLength))
        {
            // DBG("run here! ");// never
            continue;    // just in case, shouldn't really happen
        }

        // length of sequence
        temp = (seekWindowLength - 2 * overlapLength);
        // DBG(overlapLength); //352, not change with pitch
        // DBG(temp);//44.1,1942, 48,2112
        outputBuffer.putSamples(inputBuffer.ptrBegin() + channels * offset, (uint)temp);

        // Copies the end of the current sequence from 'inputBuffer' to 
        // 'midBuffer' for being mixed with the beginning of the next 
        // processing sequence and so on
        assert((offset + temp + overlapLength) <= (int)inputBuffer.numSamples());
        memcpy(pMidBuffer, inputBuffer.ptrBegin() + channels * (offset + temp),
            channels * sizeof(SAMPLETYPE) * overlapLength);

        // Remove the processed samples from the input buffer. Update
        // the difference between integer & nominal skip step to 'skipFract'
        // in order to prevent the error from accumulating over time.
        skipFract += nominalSkip;   // real skip size
        ovlSkip = (int)skipFract;   // rounded to integer skip
        skipFract -= ovlSkip;       // maintain the fraction part, i.e. real vs. integer skip
        inputBuffer.receiveSamples((uint)ovlSkip);
        // DBG(ovlSkip);// 1554 or 1555
        // DBG(skipFract);// just fraction part of the loop. ranging from 0 to 1
    }
}


// Adds 'numsamples' pcs of samples from the 'samples' memory position into
// the input of the object.
void BufferMatch::putSamples(const SAMPLETYPE* samples, uint nSamples)
{
    // Add the samples into the input buffer
    inputBuffer.putSamples(samples, nSamples);
    // Process the samples in input buffer
    processSamples();
}



/// Set new overlap length parameter & reallocate RefMidBuffer if necessary.
void BufferMatch::acceptNewOverlapLength(int newOverlapLength)
{
    int prevOvl;

    assert(newOverlapLength >= 0);
    prevOvl = overlapLength;
    overlapLength = newOverlapLength;

    if (overlapLength > prevOvl)
    {
        delete[] pMidBufferUnaligned;

        pMidBufferUnaligned = new SAMPLETYPE[overlapLength * channels + 16 / sizeof(SAMPLETYPE)];
        // ensure that 'pMidBuffer' is aligned to 16 byte boundary for efficiency
        pMidBuffer = (SAMPLETYPE*)SOUNDTOUCH_ALIGN_POINTER_16(pMidBufferUnaligned);

        clearMidBuffer();
    }
}


// Operator 'new' is overloaded so that it automatically creates a suitable instance 
// depending on if we've a MMX/SSE/etc-capable CPU available or not.
void* BufferMatch::operator new(size_t s)
{
    // Notice! don't use "new BufferMatch" directly, use "newInstance" to create a new instance instead!
    THROW_RT_ERROR("Error in BufferMatch::new: Don't use 'new BufferMatch' directly, use 'newInstance' member instead!");
    return newInstance();
}


BufferMatch* BufferMatch::newInstance()
{
    uint uExtensions;

    // uExtensions = detectCPUextensions();

    // Check if MMX/SSE instruction set extensions supported by CPU

#ifdef SOUNDTOUCH_ALLOW_MMX
    // MMX routines available only with integer sample types
    if (uExtensions & SUPPORT_MMX)
    {
        return ::new TDStretchMMX;
    }
    else
#endif // SOUNDTOUCH_ALLOW_MMX


#ifdef SOUNDTOUCH_ALLOW_SSE
        if (uExtensions & SUPPORT_SSE)
        {
            // SSE support
            return ::new TDStretchSSE;
        }
        else
#endif // SOUNDTOUCH_ALLOW_SSE

        {
            // ISA optimizations not supported, use plain C version
            return ::new BufferMatch;
        }
}



#ifdef FLOAT_SAMPLES


/// Calculates overlapInMsec period length in samples.
void BufferMatch::calculateOverlapLength(int overlapInMsec)
{
    int newOvl;

    assert(overlapInMsec >= 0);
    newOvl = (sampleRate * overlapInMsec) / 1000;
    if (newOvl < 16) newOvl = 16;

    // must be divisible by 8
    newOvl -= newOvl % 8;

    acceptNewOverlapLength(newOvl);
}


/// Calculate cross-correlation
double BufferMatch::calcCrossCorr(const float* mixingPos, const float* compare, double& anorm)
{
    // DBG("have run here");// never run here
    float corr;
    float norm;
    int i;

#ifdef ST_SIMD_AVOID_UNALIGNED
    // in SIMD mode skip 'mixingPos' positions that aren't aligned to 16-byte boundary
    if (((ulongptr)mixingPos) & 15) return -1e50;
#endif

    // hint compiler autovectorization that loop length is divisible by 8
    int ilength = (channels * overlapLength) & -8;

    corr = norm = 0;
    // Same routine for stereo and mono
    for (i = 0; i < ilength; i++)
    {
        corr += mixingPos[i] * compare[i];
        norm += mixingPos[i] * mixingPos[i];
    }

    // DBG(norm);
    anorm = norm;
    return corr / sqrt((norm < 1e-9 ? 1.0 : norm));
}


/// Update cross-correlation by accumulating "norm" coefficient by previously calculated value
double BufferMatch::calcCrossCorrAccumulate(const float* mixingPos, const float* compare, double& norm)
{
    float corr;
    int i;

    corr = 0;

    // cancel first normalizer tap from previous round
    for (i = 1; i <= channels; i++)
    {
        norm -= mixingPos[-i] * mixingPos[-i];
    }

    // hint compiler autovectorization that loop length is divisible by 8
    int ilength = (channels * overlapLength) & -8;

    // Same routine for stereo and mono
    for (i = 0; i < ilength; i++)
    {
        corr += mixingPos[i] * compare[i];
    }

    // update normalizer with last samples of this round
    for (int j = 0; j < channels; j++)
    {
        i--;
        norm += mixingPos[i] * mixingPos[i];
    }

    return corr / sqrt((norm < 1e-9 ? 1.0 : norm));
}


#endif // SOUNDTOUCH_FLOAT_SAMPLES
