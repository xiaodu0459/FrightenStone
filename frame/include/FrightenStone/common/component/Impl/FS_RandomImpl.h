/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : FS_RandomImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/25
 * @brief :
 * 
 *
 * 
 */
#ifdef __Frame_Include_FrightenStone_Common_Component_Impl_FS_Random_H__
#pragma once
#include <algorithm>

FS_NAMESPACE_BEGIN

template<typename RandValType>
struct RandomSource<RandValType, FS_RandomDefs::RAND_GEN_ALGORITHM_TYPE_MT19937>
{
    FS_Mt19937 _generator;
    RandomSource(const RandValType srandVal = static_cast<RandValType>(std::chrono::system_clock().now().time_since_epoch().count()))
        :_generator(srandVal)
    {

    }
};

template<typename RandValType>
struct RandomSource<RandValType, FS_RandomDefs::RAND_GEN_ALGORITHM_TYPE_MT19937_64>
{
    std::mt19937_64 _generator;
    RandomSource(const RandValType srandVal = static_cast<RandValType>(std::chrono::system_clock().now().time_since_epoch().count()))
        :_generator(srandVal)
    {

    }
};

template<typename RandValType, FS_RandomDefs::RAND_DIS_TYPE DisType>
inline FS_Random<RandValType, DisType>::FS_Random()
{
}

template<typename RandValType , FS_RandomDefs::RAND_DIS_TYPE DisType>
inline FS_Random<RandValType, DisType>::FS_Random(RandValType minVal, RandValType maxVal)
    :_distributor(std::min<RandValType>(minVal, maxVal), std::max<RandValType>(minVal, maxVal))
{
}

template<typename RandValType , FS_RandomDefs::RAND_DIS_TYPE DisType>
inline FS_Random<RandValType, DisType>::~FS_Random()
{
}

template<typename RandValType , FS_RandomDefs::RAND_DIS_TYPE DisType>
inline RandValType FS_Random<RandValType, DisType>::operator ()(MT1993764RandSrc &randomSrc)
{
     return  static_cast<RandValType>(_distributor._generator(randomSrc._generator));
}

template<typename RandValType, FS_RandomDefs::RAND_DIS_TYPE DisType>
inline RandValType FS_Random<RandValType, DisType>::operator ()(MT19937RandSrc &randomSrc)
{
    return  static_cast<RandValType>(_distributor._generator(randomSrc._generator));
}
#pragma region dstributor/part template
template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_SMALLINT>
{
    std::uniform_int_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_INT>
{
    std::uniform_int_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_REAL>
{
    std::uniform_real_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_BERNOULLI>
{
    std::bernoulli_distribution _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_GEOMETRIC>
{
    std::geometric_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_EXPONENTIAL>
{
    std::exponential_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_NORMAL>
{
    std::normal_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_LOGNORMAL>
{
    std::lognormal_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_BINOMIAL>
{
    std::binomial_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_CAUCHY>
{
    std::cauchy_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, FS_RandomDefs::RAND_DIS_TYPE_DISCRETE>
{
    std::discrete_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};
#pragma endregion

FS_NAMESPACE_END

#pragma endregion

#endif // __Frame_Include_FrightenStone_Common_Component_Impl_FS_Random_H__

