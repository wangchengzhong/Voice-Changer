#pragma once
#pragma once
#pragma once

#include <juce_core/juce_core.h>
#include <memory>
#include <cassert>

//#define DEBUGLOG

template<typename ElementType = double>
struct BlockCircularBufferForVC final
{
	BlockCircularBufferForVC() = default;

	BlockCircularBufferForVC(long newSize)
	{
		setSize(newSize, true);
	}

	void setReadHopSize(int hopSize)
	{
		readHopSize = hopSize;
	}

	auto getReadHopSize() const
	{
		return readHopSize;
	}

	void setWriteHopSize(int hopSize)
	{
		writeHopSize = hopSize;
	}

	auto getWriteHopSize() const
	{
		return writeHopSize;
	}

	auto getReadIndex() const
	{
		return readIndex;
	}

	auto getWriteIndex() const
	{
		return writeIndex;
	}

	void setSize(long newSize, bool shouldClear = false)
	{
		if (newSize == length)
		{
			if (shouldClear)
				reset();

			return;
		}

		block.allocate(newSize, shouldClear);
		length = newSize;
		writeIndex = readIndex = 0;
	}

	void setEnableLogging(const char* const bufferName, bool enabled)
	{
		name = bufferName;
	}

	void reset()
	{
		block.clear(length);
		writeIndex = readIndex = 0;
	}

	// 从内部缓冲区读取样本到目标音频块
	// 如果接近内部缓冲区边界，则执行读取的换行
	void read(double* const destBuffer, const long destLength)
	{
		const auto firstReadAmount = readIndex + destLength >= length ?
			length - readIndex : destLength;

		assert(destLength <= length);
		assert(firstReadAmount <= destLength);

		const auto internalBuffer = block.getData();
		assert(internalBuffer != destBuffer);

		memcpy(destBuffer, internalBuffer + readIndex, sizeof(ElementType) * firstReadAmount);

		if (firstReadAmount < destLength)
		{
			memcpy(destBuffer + firstReadAmount, internalBuffer, sizeof(ElementType) *
				(static_cast<unsigned long long>(destLength) - firstReadAmount));
		}

		readIndex += readHopSize != 0 ? readHopSize : destLength;
		readIndex %= length;
	}
	void read(float* const destBuffer, const long destLength)
	{
		const auto firstReadAmount = readIndex + destLength >= length ?
			length - readIndex : destLength;

		assert(destLength <= length);
		assert(firstReadAmount <= destLength);

		const auto internalBuffer = block.getData();
		assert(internalBuffer != destBuffer);

		for(int i = 0; i < firstReadAmount; i++)
		{
			destBuffer[i] = internalBuffer[i + readIndex];
		}
		//memcpy(destBuffer, internalBuffer + readIndex, sizeof(ElementType) * firstReadAmount);

		if (firstReadAmount < destLength)
		{
			for (int i = 0; i < static_cast<unsigned long long>(destLength) - firstReadAmount; i++)
			{
				destBuffer[i + firstReadAmount] = internalBuffer[i];
			}
			//memcpy(destBuffer + firstReadAmount, internalBuffer, sizeof(ElementType) *
			//	(static_cast<unsigned long long>(destLength) - firstReadAmount));
		}

		readIndex += readHopSize != 0 ? readHopSize : destLength;
		readIndex %= length;
	}

	// 将原音频块中的样本写入内部缓冲区
	void write(const float* sourceBuffer, const long sourceLength)
	{
		const auto firstWriteAmount = writeIndex + sourceLength >= length ?
			length - writeIndex : sourceLength;

		auto internalBuffer = block.getData();
		assert(internalBuffer != sourceBuffer);
		for(int i = 0; i < firstWriteAmount; i++)
		{
			internalBuffer[i + writeIndex] = sourceBuffer[i];
		}
		//memcpy(internalBuffer + writeIndex, sourceBuffer, sizeof(ElementType) * firstWriteAmount);

		if (firstWriteAmount < sourceLength)
		{
			for (int i = 0; i < static_cast<unsigned long long>(sourceLength) - firstWriteAmount; i++)
			{
				internalBuffer[i] = sourceBuffer[i + firstWriteAmount];
			}
			//memcpy(internalBuffer, sourceBuffer + firstWriteAmount, sizeof(ElementType) *
			//	(static_cast<unsigned long long>(sourceLength) - firstWriteAmount));
		}

		writeIndex += writeHopSize != 0 ? writeHopSize : sourceLength;
		writeIndex %= length;

		latestDataIndex = writeIndex + sourceLength % length;

	}


	void overlapWrite(ElementType* const sourceBuffer, const long sourceLength)
	{
		/*详见论文6.3.2：
		首先，需将理论交叠尺寸与实际送入音频块比较取较小值，确定实际交叠尺寸。先
			写入重叠部分，后单独写入实际送入音频块与缓冲未交叠的部分。这两次写入都
			需要检查是否到达环形区末尾。若到达末尾，需再细分成两步写入*/
		const int writeIndexDifference = getDifferenceBetweenIndexes(writeIndex, latestDataIndex, length);
		const int overlapSampleCount = sourceLength - writeHopSize;
		const auto overlapAmount = std::min(writeIndexDifference, overlapSampleCount);

		//DBG("writeIndexDifference: " << writeIndexDifference << ", overlapSampleCount: " << overlapSampleCount);
		auto tempWriteIndex = writeIndex;
		auto firstWriteAmount = writeIndex + overlapAmount > length ?
			length - writeIndex : overlapAmount;
		//DBG("firstWriteAmout: " << firstWriteAmount << "\n");

		auto internalBuffer = block.getData();

		juce::FloatVectorOperations::add(internalBuffer + writeIndex, sourceBuffer, firstWriteAmount);

		if (firstWriteAmount < overlapAmount)
		{
			juce::FloatVectorOperations::add(internalBuffer, sourceBuffer + firstWriteAmount, overlapAmount - firstWriteAmount);
		}

		tempWriteIndex += overlapAmount;
		tempWriteIndex %= length;

		const auto remainingElements = sourceLength - overlapAmount;
		firstWriteAmount = tempWriteIndex + remainingElements > length ?
			length - tempWriteIndex : remainingElements;

		memcpy(internalBuffer + tempWriteIndex, sourceBuffer + overlapAmount, sizeof(ElementType) *
			firstWriteAmount);

		if (firstWriteAmount < remainingElements)
		{
			memcpy(internalBuffer, sourceBuffer + overlapAmount + firstWriteAmount, sizeof(ElementType) *
				(remainingElements - static_cast<unsigned long long>(firstWriteAmount)));
		}
		// DBG(writeHopSize);
		// DBG(readHopSize);
		latestDataIndex = (writeIndex + sourceLength) % length;
		writeIndex += writeHopSize;
		writeIndex %= length;
	}



private:
	int getDifferenceBetweenIndexes(int index1, int index2, int bufferLength)
	{
		return (index1 <= index2) ? index2 - index1 : bufferLength - index1 + index2;
	}

private:
	juce::HeapBlock<double> block;
	long writeIndex = 0;
	long readIndex = 0;
	long length = 0;
	long latestDataIndex = 0;
	int writeHopSize = 0;
	int readHopSize = 0;


#ifdef DEBUG
	const char* name = "";
#endif
};