#pragma once
#include"VoiceConversionImpl.h"
#include"vchsm/convert_C.h"

#include"speex/global_speex_resampler.h"

#include"ppl.h"

#define TDStretch_H
#include"JuceHeader.h"
#include <stddef.h>

#include "FIFOSamplePipeForVC.h"
#include"FIFOBuffer.h"
#define DEFAULT_SEQUENCE_MS         USE_AUTO_SEQUENCE_LEN

#define USE_AUTO_SEQUENCE_LEN       0

#define DEFAULT_SEEKWINDOW_MS       USE_AUTO_SEEKWINDOW_LEN

#define USE_AUTO_SEEKWINDOW_LEN     0

///重叠长度以毫秒为单位。切片的声音序列混合在一起，
///为了形成连续的声音流，定义两个声音流的持续时间
///让连续序列彼此重叠。
///增加该值会增加计算负担，反之也会增加。
#define DEFAULT_OVERLAP_MS      8

    class BufferMatch : public FIFOProcessor
    {
    protected:

        SpeexResamplerState* upResampler;
        SpeexResamplerState* downResampler;
        int err;
        spx_uint32_t spxUpSize;
        spx_uint32_t spxDownSize;

        HSMModel model;
        const char* modelFile{ "D:/Model.dat" };
        
        int channels;
        int sampleReq;

        int overlapLength;
        int seekLength;
        int seekWindowLength;
        int overlapDividerBitsNorm;
        int overlapDividerBitsPure;
        int slopingDivider;
        int sampleRate;
        int sequenceMs;
        int seekWindowMs;
        int overlapMs;

        unsigned long maxnorm;
        float maxnormf;

        double nominalSkip;
        double skipFract;

        bool bQuickSeek;
        bool isBeginning;

        SAMPLETYPE* pMidBuffer;
        SAMPLETYPE* pMidBufferUnaligned;

        FIFOSampleBuffer outputBuffer;
        FIFOSampleBuffer inputBuffer;

        std::vector<SAMPLETYPE> vcOrigBuffer;
        std::vector<SAMPLETYPE> vcConvertedBuffer;
        std::vector<SAMPLETYPE> vcBuffer;
        int bufferLength{ 15000 };
        Eigen::TRowVectorX initializeBuffer;
        Eigen::RowVectorXi pms;
        PicosStructArray picos;
        
        Eigen::TRowVectorX f0s;

		std::unique_ptr<VoiceConversionImpl> pVcImpl;

    	void acceptNewOverlapLength(int newOverlapLength);

        virtual void clearCrossCorrState();
        void calculateOverlapLength(int overlapMs);

        virtual double calcCrossCorr(const SAMPLETYPE* mixingPos, const SAMPLETYPE* compare, double& norm);
        virtual double calcCrossCorrAccumulate(const SAMPLETYPE* mixingPos, const SAMPLETYPE* compare, double& norm);

        virtual int seekBestOverlapPositionFull(const SAMPLETYPE* refPos);
        virtual int seekBestOverlapPositionQuick(const SAMPLETYPE* refPos);
        virtual int seekBestOverlapPosition(const SAMPLETYPE* refPos);

        virtual void overlapMono(SAMPLETYPE* output, const SAMPLETYPE* input) const;

        void clearMidBuffer();
        void overlap(SAMPLETYPE* output, const SAMPLETYPE* input, uint ovlPos) const;

        void calcSeqParameters();
        
        void adaptNormalizer();

        // 实际的处理接口
        void processSamples();

    public:
        BufferMatch(int sampleRate, HSMModel model);
        virtual ~BufferMatch();

        /// 新建一个实例
        static void* operator new(size_t s, int sampleRate, HSMModel& model);
        /// 重载new
        static BufferMatch* newInstance(int sampleRate, HSMModel model);
        /// Returns the output buffer object
        FIFOSamplePipe* getOutput() { return &outputBuffer; };
        ///返回输入缓冲区对象
        FIFOSamplePipe* getInput() { return &inputBuffer; };

        /// 设定新的样本密度
        void setTempo(double newTempo);

        /// 清零输入输出
        virtual void clear();
        /// 清零输入
        void clearInput();
        void setChannels(int numChannels);
        /// 使用快速互相关搜索选项
        void enableQuickSeek(bool enable);
        /// 是否允许快速互相关搜索
        bool isQuickSeekEnabled() const;

        /// 常规参数
        void setParameters(int sampleRate,          ///(Hz)
            int sequenceMS = -1,     ///一次处理的样本点数 (ms)
            int seekwindowMS = -1,   ///寻找最佳匹配的长度(ms)
            int overlapMS = -1       ///序列叠加长度 (ms)
        );
        void setSequenceLength(int sequenceLength);

        ///获取常规参数
        void getParameters(int* pSampleRate, int* pSequenceMs, int* pSeekWindowMs, int* pOverlapMs) const;

        /// 处理样本点
        virtual void putSamples(
            const SAMPLETYPE* samples,  ///< 输入样本点数
            uint numSamples                         ///单通道音频数量
        );

        // 返回触发处理批次的标称输入样本要求
        int getInputSampleReq() const
        {
            return (int)(nominalSkip + 0.5);
        }

        ///运行处理一次时返回理论输出样本数目
        int getOutputBatchSize() const
        {
            return seekWindowLength - overlapLength;
        }

        /// 返回近似的初始输入输出延迟
        int getLatency() const
        {
            return sampleReq;
        }
    };


