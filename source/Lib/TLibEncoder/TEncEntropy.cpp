/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncEntropy.cpp
    \brief    entropy encoder class
*/

#include "TEncEntropy.h"
#include "TLibCommon/TypeDef.h"

//! \ingroup TLibEncoder
//! \{

Void TEncEntropy::setEntropyCoder ( TEncEntropyIf* e, TComSlice* pcSlice )
{
  m_pcEntropyCoderIf = e;
  m_pcEntropyCoderIf->setSlice ( pcSlice );
}

Void TEncEntropy::encodeSliceHeader ( TComSlice* pcSlice )
{
  m_pcEntropyCoderIf->codeSliceHeader( pcSlice );
  return;
}

Void TEncEntropy::encodeTerminatingBit      ( UInt uiIsLast )
{
  m_pcEntropyCoderIf->codeTerminatingBit( uiIsLast );
  
  return;
}

Void TEncEntropy::encodeSliceFinish()
{
  m_pcEntropyCoderIf->codeSliceFinish();
}

Void TEncEntropy::encodeSEI(const SEI& sei)
{
  m_pcEntropyCoderIf->codeSEI(sei);
  return;
}

Void TEncEntropy::encodePPS( TComPPS* pcPPS )
{
  m_pcEntropyCoderIf->codePPS( pcPPS );
  return;
}

Void TEncEntropy::encodeSPS( TComSPS* pcSPS )
{
  m_pcEntropyCoderIf->codeSPS( pcSPS );
  return;
}

Void TEncEntropy::encodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  m_pcEntropyCoderIf->codeSkipFlag( pcCU, uiAbsPartIdx );
}

Int TEncEntropy::lengthGolomb(int coeffVal, int k)
{
  int m = 2 << (k - 1);
  int q = coeffVal / m;
  if(coeffVal != 0)
  {
    return(q + 2 + k);
  }
  else
  {
    return(q + 1 + k);
  }
}

/** encode merge flag
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiPUIdx
 * \returns Void
 */
Void TEncEntropy::encodeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx )
{ 
  // at least one merge candidate exists
  m_pcEntropyCoderIf->codeMergeFlag( pcCU, uiAbsPartIdx );
}

/** encode merge index
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiPUIdx
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
    assert( pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N );
  }

  UInt uiNumCand = MRG_MAX_NUM_CANDS;
  if ( uiNumCand > 1 )
  {
    m_pcEntropyCoderIf->codeMergeIndex( pcCU, uiAbsPartIdx );
  }
}

/** encode prediction mode
 * \param pcCU
 * \param uiAbsPartIdx
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  if ( pcCU->getSlice()->isIntra() )
  {
    return;
  }
 
  m_pcEntropyCoderIf->codePredMode( pcCU, uiAbsPartIdx );
}

// Split mode
Void TEncEntropy::encodeSplitFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  m_pcEntropyCoderIf->codeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

/** encode partition size
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \param bRD
 * \returns Void
 */
Void TEncEntropy::encodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  m_pcEntropyCoderIf->codePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

Void TEncEntropy::xEncodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 )
{
  const UInt uiSubdiv = pcCU->getTransformIdx( uiAbsPartIdx ) + pcCU->getDepth( uiAbsPartIdx ) > uiDepth;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  {//CABAC
    if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
    {
      assert( uiSubdiv );
    }
#ifdef MOT_TUPU_MAXDEPTH1
    else if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER && (pcCU->getPartitionSize(uiAbsPartIdx) != SIZE_2Nx2N) && uiDepth == pcCU->getDepth(uiAbsPartIdx) &&  (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) )
    {
      if ( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
      {
        assert( uiSubdiv );
      }
      else
      {
        assert(!uiSubdiv );
      }
    }
#endif
    else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
    {
      assert( uiSubdiv );
    }
    else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      assert( !uiSubdiv );
    }
    else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
    {
      assert( !uiSubdiv );
    }
    else
    {
      assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
      m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSubdiv, uiDepth );
    }
  }

  {
    if( uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
    {
      const UInt uiTrDepthCurr = uiDepth - pcCU->getDepth( uiAbsPartIdx );
      const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepthCurr == 0;
      if( bFirstCbfOfCU || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) )
        {
          if ( uiInnerQuadIdx == 3 && uiUCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
          {
            uiUCbfFront3++;
          }
          else
          {
            m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr );
            uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr );
          }
        }
        if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) )
        {
          if ( uiInnerQuadIdx == 3 && uiVCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize()  )
          {
            uiVCbfFront3++;
          }
          else
          {
            m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr );
            uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr );
          }
        }
      }
      else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr - 1 ) );
        assert( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr ) == pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr - 1 ) );
        
        uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepthCurr );
        uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepthCurr );
      }
    }
    
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      
      UInt uiCurrentCbfY = 0;
      UInt uiCurrentCbfU = 0;
      UInt uiCurrentCbfV = 0;
      
      xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
      uiAbsPartIdx += uiQPartNum;
      xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 1, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
      uiAbsPartIdx += uiQPartNum;
      xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 2, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
      uiAbsPartIdx += uiQPartNum;
      xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 3, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
      
      uiYCbfFront3 += uiCurrentCbfY;
      uiUCbfFront3 += uiCurrentCbfU;
      uiVCbfFront3 += uiCurrentCbfV;
    }
    else
    {
      {
        DTRACE_CABAC_VL( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tTrIdx: abspart=" );
        DTRACE_CABAC_V( uiAbsPartIdx );
        DTRACE_CABAC_T( "\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttrdepth=" );
        DTRACE_CABAC_V( pcCU->getTransformIdx( uiAbsPartIdx ) );
        DTRACE_CABAC_T( "\n" );
      }
      UInt uiLumaTrMode, uiChromaTrMode;
      pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
      if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
      {
        assert( pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, 0 ) );
        //      printf( "saved one bin! " );
      }
      else
      {
#if CBF_CODING_SKIP_COND_FIX
        const UInt uiLog2CUSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()] + 2 - pcCU->getDepth( uiAbsPartIdx );
        if ( pcCU->getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && uiInnerQuadIdx == 3 && uiYCbfFront3 == 0 && uiUCbfFront3 == 0 && uiVCbfFront3 == 0
            && ( uiLog2CUSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() + 1 || uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() ) )
#else
        if ( pcCU->getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && uiInnerQuadIdx == 3 && uiYCbfFront3 == 0 && uiUCbfFront3 == 0 && uiVCbfFront3 == 0
            && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode ) )
#endif
        {      
          uiYCbfFront3++;
        }    
        else
        {
          m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
          uiYCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
        }
      }
      
    }
  }
}

// transform index
Void TEncEntropy::encodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD )
{
  assert( !bRD ); // parameter bRD can be removed
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\n" )
  UInt temp = 0;
  UInt temp1 = 0;
  UInt temp2 = 0;
  xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0, temp, temp1, temp2 );
}

// Intra direction for Luma
Void TEncEntropy::encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  m_pcEntropyCoderIf->codeIntraDirLumaAng( pcCU, uiAbsPartIdx );
}

// Intra direction for Chroma
Void TEncEntropy::encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  m_pcEntropyCoderIf->codeIntraDirChroma( pcCU, uiAbsPartIdx );
}

Void TEncEntropy::encodePredInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  PartSize eSize = pcCU->getPartitionSize( uiAbsPartIdx );
  
  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    if( eSize == SIZE_NxN )                                         // if it is NxN size, encode 4 intra directions.
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      // if it is NxN size, this size might be the smallest partition size.
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx                  );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset   );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*2 );
      encodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*3 );
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
    else                                                              // if it is not NxN size, encode 1 intra directions
    {
      encodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx );
      encodeIntraDirModeChroma( pcCU, uiAbsPartIdx, bRD );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    encodePUWise( pcCU, uiAbsPartIdx, bRD );
  }
}

/** encode motion information for every PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param bRD
 * \returns Void
 * This function is called only if merge is enabled.
 */
Void TEncEntropy::encodePUWise( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD )
{
  if ( bRD )
  {
    uiAbsPartIdx = 0;
  }
  
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  UInt uiNumPU = ( ePartSize == SIZE_2Nx2N ? 1 : ( ePartSize == SIZE_NxN ? 4 : 2 ) );
  UInt uiDepth = pcCU->getDepth( uiAbsPartIdx );
  UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;

  for ( UInt uiPartIdx = 0, uiSubPartIdx = uiAbsPartIdx; uiPartIdx < uiNumPU; uiPartIdx++, uiSubPartIdx += uiPUOffset )
  {
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      {
        encodeMergeFlag( pcCU, uiSubPartIdx, uiPartIdx );
      }
    }
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
      encodeMergeIndex( pcCU, uiSubPartIdx, uiPartIdx );
    }
    else
    {
      encodeInterDirPU( pcCU, uiSubPartIdx );
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          encodeRefFrmIdxPU ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
          encodeMvdPU       ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
          encodeMVPIdxPU    ( pcCU, uiSubPartIdx, RefPicList( uiRefListIdx ) );
        }
      }
    }
  }

  return;
}

Void TEncEntropy::encodeInterDirPU( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if ( !pcCU->getSlice()->isInterB() )
  {
    return;
  }

  m_pcEntropyCoderIf->codeInterDir( pcCU, uiAbsPartIdx );
  return;
}

/** encode reference frame index for a PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param eRefList
 * \returns Void
 */
Void TEncEntropy::encodeRefFrmIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C)>0 && pcCU->getInterDir( uiAbsPartIdx ) != 3)
  {
    if ((eRefList== REF_PIC_LIST_1) || ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_C ) == 1 ) )
    {
      return;
    }

    if ( pcCU->getSlice()->getNumRefIdx ( REF_PIC_LIST_C ) > 1 )
    {
      m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, RefPicList(pcCU->getInterDir( uiAbsPartIdx )-1) );
    }

  }
  else
  {
    if ( ( pcCU->getSlice()->getNumRefIdx( eRefList ) == 1 ) )
    {
      return;
    }

    if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
    {
      m_pcEntropyCoderIf->codeRefFrmIdx( pcCU, uiAbsPartIdx, eRefList );
    }
  }

  return;
}

/** encode motion vector difference for a PU block
 * \param pcCU
 * \param uiAbsPartIdx
 * \param eRefList
 * \returns Void
 */
Void TEncEntropy::encodeMvdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert( !pcCU->isIntra( uiAbsPartIdx ) );

  if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
  {
    m_pcEntropyCoderIf->codeMvd( pcCU, uiAbsPartIdx, eRefList );
  }
  return;
}

Void TEncEntropy::encodeMVPIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  if ( (pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList )) && (pcCU->getAMVPMode(uiAbsPartIdx) == AM_EXPL) )
  {
    m_pcEntropyCoderIf->codeMVPIdx( pcCU, uiAbsPartIdx, eRefList );
  }

  return;
}

Void TEncEntropy::encodeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  m_pcEntropyCoderIf->codeQtCbf( pcCU, uiAbsPartIdx, eType, uiTrDepth );
}

Void TEncEntropy::encodeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcEntropyCoderIf->codeTransformSubdivFlag( uiSymbol, uiCtx );
}

Void TEncEntropy::encodeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  m_pcEntropyCoderIf->codeQtRootCbf( pcCU, uiAbsPartIdx );
}

// texture
#if TU_LEVEL_COEFF_INTERLEAVE
Void TEncEntropy::xEncodeCoeff( TComDataCU* pcCU, UInt uiLumaOffset, UInt uiChromaOffset, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx )
#else
Void TEncEntropy::xEncodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType )
#endif
{
#if TU_LEVEL_COEFF_INTERLEAVE
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
  UInt uiCbfY = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrIdx );
  UInt uiCbfU = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
  UInt uiCbfV = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );

  if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
    if( ( uiAbsPartIdx % uiQPDiv ) == 0 )
    {
      m_uiBakAbsPartIdx   = uiAbsPartIdx;
      m_uiBakChromaOffset = uiChromaOffset;
    }
    else if( ( uiAbsPartIdx % uiQPDiv ) == (uiQPDiv - 1) )
    {
      uiCbfU = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
      uiCbfV = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
    }
  }

  if ( uiCbfY || uiCbfU || uiCbfV )
#else
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
#endif
  {
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
#if TU_LEVEL_COEFF_INTERLEAVE
    const UInt uiStopTrMode = uiLumaTrMode;
#else
    const UInt uiStopTrMode = eType == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
#endif
    
    assert(1); // as long as quadtrees are not used for residual transform
    
    if( uiTrIdx == uiStopTrMode )
    {
#if TU_LEVEL_COEFF_INTERLEAVE
      if( pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrIdx ) )
      {
#if NSQT_MOD
        Int trWidth = uiWidth;
        Int trHeight = uiHeight;
        pcCU->getNSQTSize( uiTrIdx, uiAbsPartIdx, trWidth, trHeight );
        m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffY()+uiLumaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_LUMA );
#else
        m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffY()+uiLumaOffset), uiAbsPartIdx, uiWidth, uiHeight, uiDepth, TEXT_LUMA );
#endif
      }

      uiWidth  >>= 1;
      uiHeight >>= 1;

      if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) == (uiQPDiv - 1) )
        {
          uiWidth  <<= 1;
          uiHeight <<= 1;
#if NSQT_MOD
          Int trWidth = uiWidth;
          Int trHeight = uiHeight;
          pcCU->getNSQTSize( uiTrIdx-1, uiAbsPartIdx, trWidth, trHeight );
#endif
          if( pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx ) )
          {
#if NSQT_MOD
            m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
#else
            m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, uiWidth, uiHeight, uiDepth, TEXT_CHROMA_U );
#endif
          }
          if( pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx ) )
          {
#if NSQT_MOD
            m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
#else
            m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, uiWidth, uiHeight, uiDepth, TEXT_CHROMA_V );
#endif
          }
        }
      }
      else
      {
#if NSQT_MOD
        Int trWidth = uiWidth;
        Int trHeight = uiHeight;
        pcCU->getNSQTSize( uiTrIdx, uiAbsPartIdx, trWidth, trHeight );
#endif
        if( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx ) )
        {
#if NSQT_MOD
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+uiChromaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
#else
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCb()+uiChromaOffset), uiAbsPartIdx, uiWidth, uiHeight, uiDepth, TEXT_CHROMA_U );
#endif
        }
        if( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx ) )
        {
#if NSQT_MOD
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+uiChromaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
#else
          m_pcEntropyCoderIf->codeCoeffNxN( pcCU, (pcCU->getCoeffCr()+uiChromaOffset), uiAbsPartIdx, uiWidth, uiHeight, uiDepth, TEXT_CHROMA_V );
#endif
        }
      }
#else
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
#if NSQT_MOD
      Int trWidth = uiWidth;
      Int trHeight = uiHeight;
      pcCU->getNSQTSize( uiTrIdx, uiAbsPartIdx, trWidth, trHeight );
#endif
      if( eType != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
        {
          return;
        }
        uiWidth  <<= 1;
        uiHeight <<= 1;
#if NSQT_MOD
      trWidth = uiWidth;
      trHeight = uiHeight;
      pcCU->getNSQTSize( uiTrIdx-1, uiAbsPartIdx, trWidth, trHeight );
#endif
      }
#if NSQT_MOD
      m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, trWidth, trHeight, uiDepth, eType );
#else
      m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType );
#endif
#endif
    }
    else
    {
      {
        DTRACE_CABAC_VL( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing down\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttridx=" );
        DTRACE_CABAC_V( uiTrIdx );
        DTRACE_CABAC_T( "\n" );
      }
      if( uiCurrTrIdx <= uiTrIdx )
        assert(1);
      
      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;
      
      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;
      
      {
#if TU_LEVEL_COEFF_INTERLEAVE
        xEncodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx );
        uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;

        xEncodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx );
        uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;

        xEncodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx );
        uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;

        xEncodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx );
#else
        xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xEncodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType );
#endif
      }
      {
        DTRACE_CABAC_VL( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing up\n" );
      }
    }
  }
}

#if !TU_LEVEL_COEFF_INTERLEAVE
Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, TCoeff* pCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiMaxTrMode, UInt uiTrMode, TextType eType )
{
  xEncodeCoeff( pcCU, pCoeff, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, uiTrMode, uiMaxTrMode, eType );
}
#endif

/** encode coefficients
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \param uiWidth
 * \param uiHeight
 */
Void TEncEntropy::encodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
  
  UInt uiLumaTrMode, uiChromaTrMode;
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
  
  if( pcCU->isIntra(uiAbsPartIdx) )
  {
    DTRACE_CABAC_VL( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
    DTRACE_CABAC_V( uiDepth )
    DTRACE_CABAC_T( "\n" )
    UInt temp = 0;
    UInt temp1 = 0;
    UInt temp2 = 0;
    xEncodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0, temp, temp1, temp2 );
    
  }
  else
  {
      if( !(pcCU->getMergeFlag( uiAbsPartIdx ) && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N ) )
      {
        m_pcEntropyCoderIf->codeQtRootCbf( pcCU, uiAbsPartIdx );
      }
      if ( !pcCU->getQtRootCbf( uiAbsPartIdx ) )
      {
        return;
      }
    
    encodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
  }
  
#if TU_LEVEL_COEFF_INTERLEAVE
  xEncodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, 0, uiLumaTrMode );
#else
  xEncodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA     );
  xEncodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U );
  xEncodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V );
#endif
}

Void TEncEntropy::encodeCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType )
{ // This is for Transform unit processing. This may be used at mode selection stage for Inter.
  m_pcEntropyCoderIf->codeCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiTrWidth, uiTrHeight, uiDepth, eType );
}

#if NSQT_DIAG_SCAN
Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType)
#else
Void TEncEntropy::estimateBit (estBitsSbacStruct* pcEstBitsSbac, UInt uiWidth, TextType eTType)
#endif
{
#if !NSQT_DIAG_SCAN
  UInt uiCTXIdx;
  
  switch(uiWidth)
  {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
  }
#endif
  
  eTType = eTType == TEXT_LUMA ? TEXT_LUMA : TEXT_CHROMA;
  
#if NSQT_DIAG_SCAN
  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, width, height, eTType );
#else
  m_pcEntropyCoderIf->estBit ( pcEstBitsSbac, uiCTXIdx, eTType );
#endif
}

Int TEncEntropy::countNonZeroCoeffs( TCoeff* pcCoef, UInt uiSize )
{
  Int count = 0;
  
#if NSQT_DIAG_SCAN
  for ( Int i = 0; i < uiSize; i++ )
#else
  for ( Int i = 0; i < uiSize * uiSize; i++ )
#endif
  {
    count += pcCoef[i] != 0;
  }
  
  return count;
}

//! \}
