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

/** \file     TAppEncCfg.cpp
    \brief    Handle encoder configuration parameters
*/

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include "TLibCommon/TComRom.h"
#include "TAppEncCfg.h"
#include "TAppCommon/program_options_lite.h"

#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

//! \ingroup TAppEncoder
//! \{

/* configuration helper funcs */
void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg);
void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg);

// ====================================================================================================================
// Local constants
// ====================================================================================================================

/// max value of source padding size
/** \todo replace it by command line option
 */
#define MAX_PAD_SIZE                16

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppEncCfg::TAppEncCfg()
{
}

TAppEncCfg::~TAppEncCfg()
{
}

Void TAppEncCfg::create()
{
}

Void TAppEncCfg::destroy()
{
}

#if G1002_RPS
std::istringstream &operator>>(std::istringstream &in, GOPEntry &entry)     //input
{
  memset(&entry, 0, sizeof(entry));
  in>>entry.m_iSliceType;
  in>>entry.m_iPOC;
  in>>entry.m_iQPOffset;
  in>>entry.m_iQPFactor;
  in>>entry.m_iTemporalId;
  in>>entry.m_iRefBufSize;
  in>>entry.m_bRefPic;
  in>>entry.m_iNumRefPics;
  for ( Int i = 0; i < entry.m_iNumRefPics; i++ )
  {
    in>>entry.m_aiReferencePics[i];
  }
#if INTER_RPS_PREDICTION
  in>>entry.m_bInterRPSPrediction;
  if (entry.m_bInterRPSPrediction)
  {
    in>>entry.m_iDeltaRIdxMinus1;
    in>>entry.m_iDeltaRPS;
    in>>entry.m_iNumRefIdc;
    for ( Int i = 0; i < entry.m_iNumRefIdc; i++ )
    {
      in>>entry.m_aiRefIdc[i];
    }
  }
#endif
  return in;
}
#endif
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
    \param  argv        array of arguments
    \retval             true when success
 */
Bool TAppEncCfg::parseCfg( Int argc, Char* argv[] )
{
  bool do_help = false;
  
  string cfg_InputFile;
  string cfg_BitstreamFile;
  string cfg_ReconFile;
  po::Options opts;
  opts.addOptions()
  ("help", do_help, false, "this help text")
  ("c", po::parseConfigFile, "configuration file name")
  
  /* File, I/O and source parameters */
  ("InputFile,i",     cfg_InputFile,     string(""), "original YUV input file name")
  ("BitstreamFile,b", cfg_BitstreamFile, string(""), "bitstream output file name")
  ("ReconFile,o",     cfg_ReconFile,     string(""), "reconstructed YUV output file name")
  
  ("SourceWidth,-wdt",      m_iSourceWidth,  0, "Source picture width")
  ("SourceHeight,-hgt",     m_iSourceHeight, 0, "Source picture height")
  ("HorizontalPadding,-pdx",m_aiPad[0],      0, "horizontal source padding size")
  ("VerticalPadding,-pdy",  m_aiPad[1],      0, "vertical source padding size")
  ("PAD",                   m_bUsePAD,   false, "automatic source padding of multiple of 16" )
  ("FrameRate,-fr",         m_iFrameRate,        0, "Frame rate")
  ("FrameSkip,-fs",         m_FrameSkip,         0u, "Number of frames to skip at start of input YUV")
  ("FramesToBeEncoded,f",   m_iFrameToBeEncoded, 0, "number of frames to be encoded (default=all)")
  ("FrameToBeEncoded",      m_iFrameToBeEncoded, 0, "depricated alias of FramesToBeEncoded")
  
  /* Unit definition parameters */
  ("MaxCUWidth",          m_uiMaxCUWidth,  64u)
  ("MaxCUHeight",         m_uiMaxCUHeight, 64u)
  /* todo: remove defaults from MaxCUSize */
  ("MaxCUSize,s",         m_uiMaxCUWidth,  64u, "max CU size")
  ("MaxCUSize,s",         m_uiMaxCUHeight, 64u, "max CU size")
  ("MaxPartitionDepth,h", m_uiMaxCUDepth,   4u, "CU depth")
  
  ("QuadtreeTULog2MaxSize", m_uiQuadtreeTULog2MaxSize, 6u)
  ("QuadtreeTULog2MinSize", m_uiQuadtreeTULog2MinSize, 2u)
  
  ("QuadtreeTUMaxDepthIntra", m_uiQuadtreeTUMaxDepthIntra, 1u)
  ("QuadtreeTUMaxDepthInter", m_uiQuadtreeTUMaxDepthInter, 2u)
  
  /* Coding structure paramters */
  ("IntraPeriod,-ip",m_iIntraPeriod, -1, "intra period in frames, (-1: only first frame)")
  ("DecodingRefreshType,-dr",m_iDecodingRefreshType, 0, "intra refresh, (0:none 1:CDR 2:IDR)")
#if G1002_RPS
  ("MaxNumberOfReorderPictures",   m_uiMaxNumberOfReorderPictures,   4u, "Max number of reorder pictures")
  ("MaxNumberOfReferencePictures", m_uiMaxNumberOfReferencePictures, 6u, "Max number of reference pictures")
#else
  ("NumOfReference,r",       m_iNumOfReference,     1, "Number of reference (P)")
  ("NumOfReferenceB_L0,-rb0",m_iNumOfReferenceB_L0, 1, "Number of reference (B_L0)")
  ("NumOfReferenceB_L1,-rb1",m_iNumOfReferenceB_L1, 1, "Number of reference (B_L1)")
  ("GPB", m_bUseGPB, false, "generalized B instead of P in low-delay mode")
#endif
#if !G1002_RPS
  ("NRF", m_bUseNRF,  true, "non-reference frame marking in last layer")
  ("BQP", m_bUseBQP, false, "hier-P style QP assignment in low-delay mode")
#endif
#if DISABLE_4x4_INTER
  ("DisableInter4x4", m_bDisInter4x4, true, "Disable Inter 4x4")
#endif
#if NSQT
  ("NSQT", m_enableNSQT, true, "Enable non-square transforms")
#endif
#if AMP
  ("AMP", m_enableAMP, true, "Enable asymmetric motion partitions")
#endif
  /* motion options */
  ("FastSearch", m_iFastSearch, 1, "0:Full search  1:Diamond  2:PMVFAST")
  ("SearchRange,-sr",m_iSearchRange, 96, "motion search range")
  ("BipredSearchRange", m_bipredSearchRange, 4, "motion search range for bipred refinement")
  ("HadamardME", m_bUseHADME, true, "hadamard ME for fractional-pel")
  
  /* Quantization parameters */
  ("QP,q",          m_fQP,             30.0, "Qp value, if value is float, QP is switched once during encoding")
  ("RDOQ",          m_bUseRDOQ, true)
  
  ("SBACRD", m_bUseSBACRD, true, "SBAC based RD estimation")
  
  /* Deblocking filter parameters */
  ("LoopFilterDisable", m_bLoopFilterDisable, false)
  ("LoopFilterAlphaC0Offset", m_iLoopFilterAlphaC0Offset, 0)
  ("LoopFilterBetaOffset", m_iLoopFilterBetaOffset, 0 )
  
  /* Coding tools */
  ("MRG", m_bUseMRG, true, "merging of motion partitions")

  ("LMChroma", m_bUseLMChroma, true, "intra chroma prediction based on recontructed luma")

    ("ConstrainedIntraPred", m_bUseConstrainedIntraPred, false, "Constrained Intra Prediction")
  /* Misc. */
  ("SEIpictureDigest", m_pictureDigestEnabled, true, "Control generation of picture_digest SEI messages\n"
                                              "\t1: use MD5\n"
                                              "\t0: disable")
#if !G1002_RPS
#if REF_SETTING_FOR_LD
  ("UsingNewRefSetting", m_bUseNewRefSetting, false, "Use 1+X reference frame setting for LD" )
#endif
#endif

  ("FEN", m_bUseFastEnc, false, "fast encoder setting")
#if EARLY_CU_DETERMINATION
  ("ECU", m_bUseEarlyCU, false, "Early CU setting") 
#endif
#if CBF_FAST_MODE
  ("CFM", m_bUseCbfFastMode, false, "Cbf fast mode setting")
#endif
  /* Compatability with old style -1 FOO or -0 FOO options. */
  ("1", doOldStyleCmdlineOn, "turn option <name> on")
  ("0", doOldStyleCmdlineOff, "turn option <name> off")
  ;
  
#if G1002_RPS
  for(Int i=1; i<MAX_GOP+1; i++) {
    std::ostringstream cOSS;
    cOSS<<"Frame"<<i;
    opts.addOptions()(cOSS.str(), m_pcGOPList[i-1], GOPEntry());
  }
#endif
  po::setDefaults(opts);
  const list<const char*>& argv_unhandled = po::scanArgv(opts, argc, (const char**) argv);

  for (list<const char*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }
  
  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    xPrintUsage();
    return false;
  }
  
  /*
   * Set any derived parameters
   */
  /* convert std::string to c string for compatability */
  m_pchInputFile = cfg_InputFile.empty() ? NULL : strdup(cfg_InputFile.c_str());
  m_pchBitstreamFile = cfg_BitstreamFile.empty() ? NULL : strdup(cfg_BitstreamFile.c_str());
  m_pchReconFile = cfg_ReconFile.empty() ? NULL : strdup(cfg_ReconFile.c_str());
  
  // compute source padding size
  if ( m_bUsePAD )
  {
    if ( m_iSourceWidth%MAX_PAD_SIZE )
    {
      m_aiPad[0] = (m_iSourceWidth/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceWidth;
    }
    
    if ( m_iSourceHeight%MAX_PAD_SIZE )
    {
      m_aiPad[1] = (m_iSourceHeight/MAX_PAD_SIZE+1)*MAX_PAD_SIZE - m_iSourceHeight;
    }
  }
  m_iSourceWidth  += m_aiPad[0];
  m_iSourceHeight += m_aiPad[1];
  
  // handling of floating-point QP values
  // if QP is not integer, sequence is split into two sections having QP and QP+1
  m_iQP = (Int)( m_fQP );
  
#if !G1002_RPS
#if REF_SETTING_FOR_LD
    if ( m_bUseNewRefSetting )
    {
      printf( "\nwarning: new reference frame setting was originally designed for default LD setting (rateGOPSize=4), no action" );
    }
#endif
#endif
  
  // check validity of input parameters
  xCheckParameter();
  
  // set global varibles
  xSetGlobal();
  
  // print-out parameters
  xPrintParameter();
  
  return true;
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Bool confirmPara(Bool bflag, const char* message);

Void TAppEncCfg::xCheckParameter()
{
  bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
  // check range of parameters
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_iFrameToBeEncoded <= 0,                                                   "Total Number Of Frames encoded must be more than 1" );
  xConfirmPara( m_iIntraPeriod == 0,                                                        "Intra period must be more than GOP size, or -1 , not 0" );
  xConfirmPara( m_iDecodingRefreshType < 0 || m_iDecodingRefreshType > 2,                   "Decoding Refresh Type must be equal to 0, 1 or 2" );
  xConfirmPara( m_iQP < 0 || m_iQP > 51,                                                    "QP exceeds supported range (0 to 51)" );

  xConfirmPara( m_iLoopFilterAlphaC0Offset < -26 || m_iLoopFilterAlphaC0Offset > 26,        "Loop Filter Alpha Offset exceeds supported range (-26 to 26)" );
  xConfirmPara( m_iLoopFilterBetaOffset < -26 || m_iLoopFilterBetaOffset > 26,              "Loop Filter Beta Offset exceeds supported range (-26 to 26)");
  xConfirmPara( m_iFastSearch < 0 || m_iFastSearch > 2,                                     "Fast Search Mode is not supported value (0:Full search  1:Diamond  2:PMVFAST)" );
  xConfirmPara( m_iSearchRange < 0 ,                                                        "Search Range must be more than 0" );
  xConfirmPara( m_bipredSearchRange < 0 ,                                                   "Search Range must be more than 0" );
#if G1002_RPS
  xConfirmPara( m_iFrameToBeEncoded != 1 && m_iFrameToBeEncoded < 2,                        "Total Number of Frames to be encoded must be at least 2 * GOP size for the current Reference Picture Set settings");
#else
  xConfirmPara( m_iFrameToBeEncoded != 1 && m_iFrameToBeEncoded <= 1,                       "Total Number of Frames to be encoded must be larger than GOP size");
#endif
  xConfirmPara( (m_uiMaxCUWidth  >> m_uiMaxCUDepth) < 4,                                    "Minimum partition width size should be larger than or equal to 8");
  xConfirmPara( (m_uiMaxCUHeight >> m_uiMaxCUDepth) < 4,                                    "Minimum partition height size should be larger than or equal to 8");
  xConfirmPara( m_uiMaxCUWidth < 16,                                                        "Maximum partition width size should be larger than or equal to 16");
  xConfirmPara( m_uiMaxCUHeight < 16,                                                       "Maximum partition height size should be larger than or equal to 16");
  xConfirmPara( (m_iSourceWidth  % (m_uiMaxCUWidth  >> (m_uiMaxCUDepth-1)))!=0,             "Frame width should be multiple of minimum CU size");
  xConfirmPara( (m_iSourceHeight % (m_uiMaxCUHeight >> (m_uiMaxCUDepth-1)))!=0,             "Frame height should be multiple of minimum CU size");
  
  xConfirmPara( m_uiQuadtreeTULog2MinSize < 2,                                        "QuadtreeTULog2MinSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MinSize > 5,                                        "QuadtreeTULog2MinSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < 2,                                        "QuadtreeTULog2MaxSize must be 2 or greater.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize > 5,                                        "QuadtreeTULog2MaxSize must be 5 or smaller.");
  xConfirmPara( m_uiQuadtreeTULog2MaxSize < m_uiQuadtreeTULog2MinSize,                "QuadtreeTULog2MaxSize must be greater than or equal to m_uiQuadtreeTULog2MinSize.");
  xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUWidth >>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
  xConfirmPara( (1<<m_uiQuadtreeTULog2MinSize)>(m_uiMaxCUHeight>>(m_uiMaxCUDepth-1)), "QuadtreeTULog2MinSize must not be greater than minimum CU size" ); // HS
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) > ( m_uiMaxCUWidth  >> m_uiMaxCUDepth ), "Minimum CU width must be greater than minimum transform size." );
  xConfirmPara( ( 1 << m_uiQuadtreeTULog2MinSize ) > ( m_uiMaxCUHeight >> m_uiMaxCUDepth ), "Minimum CU height must be greater than minimum transform size." );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter < 1,                                                         "QuadtreeTUMaxDepthInter must be greater than or equal to 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthInter > m_uiQuadtreeTULog2MaxSize - m_uiQuadtreeTULog2MinSize + 1, "QuadtreeTUMaxDepthInter must be less than or equal to the difference between QuadtreeTULog2MaxSize and QuadtreeTULog2MinSize plus 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra < 1,                                                         "QuadtreeTUMaxDepthIntra must be greater than or equal to 1" );
  xConfirmPara( m_uiQuadtreeTUMaxDepthIntra > m_uiQuadtreeTULog2MaxSize - m_uiQuadtreeTULog2MinSize + 1, "QuadtreeTUMaxDepthIntra must be less than or equal to the difference between QuadtreeTULog2MaxSize and QuadtreeTULog2MinSize plus 1" );

  // max CU width and height should be power of 2
  UInt ui = m_uiMaxCUWidth;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Width should be 2^n");
  }
  ui = m_uiMaxCUHeight;
  while(ui)
  {
    ui >>= 1;
    if( (ui & 1) == 1)
      xConfirmPara( ui != 1 , "Height should be 2^n");
  }
  
#if G1002_RPS
  Bool bVerified_GOP=false;
  Bool bError_GOP=false;
  Int iCheckGOP=1;
  Int iNumRefs = 1;
  Int aRefList[MAX_NUM_REF_PICS+1];
  aRefList[0]=0;
  Bool bIsOK[MAX_GOP];
  for(Int i=0; i<MAX_GOP; i++) {
    bIsOK[i]=false;
  }
  Int iNumOK=0;
  m_uiMaxNumberOfReorderPictures=0;
  m_uiMaxNumberOfReferencePictures=0;
  Int iLastDisp = -1;
  m_iExtraRPSs=0;
  while(!bVerified_GOP&&!bError_GOP) 
  {
    Int iCurGOP = 0;
    Int iCurPOC = (iCheckGOP-1) + m_pcGOPList[iCurGOP].m_iPOC;
    
    if(m_pcGOPList[iCurGOP].m_iPOC<0) {
      printf("\nError: found fewer Reference Picture Sets than GOPSize\n");
      bError_GOP=true;
    }
    else {
      Bool bBeforeI = false;
      for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
      {
        Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
        if(iAbsPOC < 0)
          bBeforeI=true;
        else {
          Bool bFound=false;
          for(Int j=0; j<iNumRefs; j++) {
            if(aRefList[j]==iAbsPOC) {
              bFound=true;
              m_pcGOPList[iCurGOP].m_aiUsedByCurrPic[i]=m_pcGOPList[0].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId;
            }
          }
          if(!bFound)
          {
            printf("\nError: ref pic %d is not available for GOP frame %d\n",m_pcGOPList[iCurGOP].m_aiReferencePics[i],iCurGOP+1);
            bError_GOP=true;
          }
        }
      }
      if(!bBeforeI&&!bError_GOP)
      {
        //all ref frames were present
        if(!bIsOK[iCurGOP]) 
        {
          iNumOK++;
          bIsOK[iCurGOP]=true;
          if(iNumOK==1)
            bVerified_GOP=true;
        }
      }
      else {
        
        m_pcGOPList[1+m_iExtraRPSs]=m_pcGOPList[iCurGOP];
        Int iNewRefs=0;
        for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
        {
          Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
          if(iAbsPOC>=0)
          {
            m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[iNewRefs]=m_pcGOPList[iCurGOP].m_aiReferencePics[i];
            m_pcGOPList[1+m_iExtraRPSs].m_aiUsedByCurrPic[iNewRefs]=m_pcGOPList[iCurGOP].m_aiUsedByCurrPic[i];
            iNewRefs++;
          }
        }
        Int iNumPrefRefs = m_pcGOPList[iCurGOP].m_iRefBufSize;
        
        for(Int iOffset = -1; iOffset>-iCheckGOP; iOffset--)
        {
          //step backwards in coding order and include pictures we might find useful. 
          Int iOffGOP = (iCheckGOP-1+iOffset)%1;
          Int iOffPOC = (iCheckGOP-1+iOffset) + m_pcGOPList[iOffGOP].m_iPOC;
          if(iOffPOC>=0&&m_pcGOPList[iOffGOP].m_bRefPic&&m_pcGOPList[iOffGOP].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId) 
          {
            Bool bNewRef=false;
            for(Int i=0; i<iNumRefs; i++)
            {
              if(aRefList[i]==iOffPOC)
              {
                bNewRef=true;
              }
            }
            for(Int i=0; i<iNewRefs; i++) 
            {
              if(m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[i]==iOffPOC-iCurPOC)
              {
                bNewRef=false;
              }
            }
            if(bNewRef) 
            {
              Int iInsertPoint=iNewRefs;
              for(Int j=0; j<iNewRefs; j++)
              {
                if(m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[j]<iOffPOC-iCurPOC||m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[j]>0)
                {
                  iInsertPoint = j;
                  break;
                }
              }
              Int prev = iOffPOC-iCurPOC;
              Int prevUsed = m_pcGOPList[iOffGOP].m_iTemporalId<=m_pcGOPList[iCurGOP].m_iTemporalId;
              for(Int j=iInsertPoint; j<iNewRefs+1; j++)
              {
                Int newPrev = m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[j];
                Int newUsed = m_pcGOPList[1+m_iExtraRPSs].m_aiUsedByCurrPic[j];
                m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[j]=prev;
                m_pcGOPList[1+m_iExtraRPSs].m_aiUsedByCurrPic[j]=prevUsed;
                prevUsed=newUsed;
                prev=newPrev;
              }
              //m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[iNewRefs]=iOffPOC-iCurPOC;
              iNewRefs++;
            }
          }
          if(iNewRefs>=iNumPrefRefs)
            break;
        }
        m_pcGOPList[1+m_iExtraRPSs].m_iNumRefPics=iNewRefs;
        m_pcGOPList[1+m_iExtraRPSs].m_iPOC = iCurPOC;
#if INTER_RPS_PREDICTION
        if (m_iExtraRPSs == 0)
        {
          m_pcGOPList[1+m_iExtraRPSs].m_bInterRPSPrediction = 0;
          m_pcGOPList[1+m_iExtraRPSs].m_iNumRefIdc = 0;
        }
        else
        {
          Int rIdx =  1 + m_iExtraRPSs - 1;
          Int iRefPOC = m_pcGOPList[rIdx].m_iPOC;
          Int iRefPics = m_pcGOPList[rIdx].m_iNumRefPics;
          Int iNewIdc=0;
          for(Int i = 0; i<= iRefPics; i++) 
          {
            Int deltaPOC = ((i != iRefPics)? m_pcGOPList[rIdx].m_aiReferencePics[i] : 0);  // check if the reference abs POC is >= 0
            Int iAbsPOCref = iRefPOC+deltaPOC;
            Int iRefIdc = 0;
            for (Int j = 0; j < m_pcGOPList[1+m_iExtraRPSs].m_iNumRefPics; j++)
            {
              if ( (iAbsPOCref - iCurPOC) == m_pcGOPList[1+m_iExtraRPSs].m_aiReferencePics[j])
              {
                if (m_pcGOPList[1+m_iExtraRPSs].m_aiUsedByCurrPic[j])
                {
                  iRefIdc = 1;
                }
                else
                {
                  iRefIdc = 2;
                }
              }
            }
            m_pcGOPList[1+m_iExtraRPSs].m_aiRefIdc[iNewIdc]=iRefIdc;
            iNewIdc++;
          }
          m_pcGOPList[1+m_iExtraRPSs].m_bInterRPSPrediction = 1;  
          m_pcGOPList[1+m_iExtraRPSs].m_iNumRefIdc = iNewIdc;
          m_pcGOPList[1+m_iExtraRPSs].m_iDeltaRPS = iRefPOC - m_pcGOPList[1+m_iExtraRPSs].m_iPOC; 
          m_pcGOPList[1+m_iExtraRPSs].m_iDeltaRIdxMinus1 = 0; 
        }
#endif        
        iCurGOP=1+m_iExtraRPSs;
        m_iExtraRPSs++;
      }
      iNumRefs=0;
      for(Int i = 0; i< m_pcGOPList[iCurGOP].m_iNumRefPics; i++) 
      {
        Int iAbsPOC = iCurPOC+m_pcGOPList[iCurGOP].m_aiReferencePics[i];
        if(iAbsPOC >= 0) {
          aRefList[iNumRefs]=iAbsPOC;
          iNumRefs++;
        }
      }
      if(m_uiMaxNumberOfReferencePictures<iNumRefs)
        m_uiMaxNumberOfReferencePictures=iNumRefs;
      aRefList[iNumRefs]=iCurPOC;
      iNumRefs++;
      Int iNonDisplayed=0;
      for(Int i=0; i<iNumRefs; i++) {
        if(aRefList[i]==iLastDisp+1) {
          iLastDisp=aRefList[i];
          i=0;
        }
      }
      for(Int i=0; i<iNumRefs; i++) {
        if(aRefList[i]>iLastDisp)
          iNonDisplayed++;
      }
      if(iNonDisplayed>m_uiMaxNumberOfReorderPictures)
        m_uiMaxNumberOfReorderPictures=iNonDisplayed;
    }
    iCheckGOP++;
  }
  xConfirmPara(bError_GOP,"Invalid GOP structure given");
  for(Int i=0; i<1; i++) 
  {
    xConfirmPara(m_pcGOPList[i].m_iSliceType!='B'&&m_pcGOPList[i].m_iSliceType!='P', "Slice type must be equal to B or P");
  }
#else
#if REF_SETTING_FOR_LD
  xConfirmPara( m_bUseNewRefSetting && 1>1, "New reference frame setting was only designed for LD setting" );
#endif
#endif

#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

/** \todo use of global variables should be removed later
 */
Void TAppEncCfg::xSetGlobal()
{
  // set max CU width & height
  g_uiMaxCUWidth  = m_uiMaxCUWidth;
  g_uiMaxCUHeight = m_uiMaxCUHeight;
  
  // compute actual CU depth with respect to config depth and max transform size
  g_uiAddCUDepth  = 0;
  while( (m_uiMaxCUWidth>>m_uiMaxCUDepth) > ( 1 << ( m_uiQuadtreeTULog2MinSize + g_uiAddCUDepth )  ) ) g_uiAddCUDepth++;
  
  m_uiMaxCUDepth += g_uiAddCUDepth;
  g_uiAddCUDepth++;
  g_uiMaxCUDepth = m_uiMaxCUDepth;

  g_uiBASE_MAX     = ((1<<8)-1);
  
#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX     = g_uiBASE_MAX;
#else
  g_uiIBDI_MAX     = ((1<<8)-1);
#endif
}

Void TAppEncCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File          : %s\n", m_pchInputFile          );
  printf("Bitstream      File          : %s\n", m_pchBitstreamFile      );
  printf("Reconstruction File          : %s\n", m_pchReconFile          );
  printf("Real     Format              : %dx%d %dHz\n", m_iSourceWidth - m_aiPad[0], m_iSourceHeight-m_aiPad[1], m_iFrameRate );
  printf("Internal Format              : %dx%d %dHz\n", m_iSourceWidth, m_iSourceHeight, m_iFrameRate );
  printf("Frame index                  : %u - %d (%d frames)\n", m_FrameSkip, m_FrameSkip+m_iFrameToBeEncoded-1, m_iFrameToBeEncoded );
#if !G1002_RPS
  printf("Number of Ref. frames (P)    : %d\n", m_iNumOfReference);
  printf("Number of Ref. frames (B_L0) : %d\n", m_iNumOfReferenceB_L0);
  printf("Number of Ref. frames (B_L1) : %d\n", m_iNumOfReferenceB_L1);
  printf("Number of Reference frames   : %d\n", m_iNumOfReference);
#endif
  printf("CU size / depth              : %d / %d\n", m_uiMaxCUWidth, m_uiMaxCUDepth );
  printf("RQT trans. size (min / max)  : %d / %d\n", 1 << m_uiQuadtreeTULog2MinSize, 1 << m_uiQuadtreeTULog2MaxSize );
  printf("Max RQT depth inter          : %d\n", m_uiQuadtreeTUMaxDepthInter);
  printf("Max RQT depth intra          : %d\n", m_uiQuadtreeTUMaxDepthIntra);
  printf("Motion search range          : %d\n", m_iSearchRange );
  printf("Intra period                 : %d\n", m_iIntraPeriod );
  printf("Decoding refresh type        : %d\n", m_iDecodingRefreshType );
  printf("QP                           : %5.2f\n", m_fQP );
#if DISABLE_4x4_INTER
  printf("DisableInter4x4              : %d\n", m_bDisInter4x4);  
#endif
  printf("\n");
  
  printf("TOOL CFG: ");
  printf("HAD:%d ", m_bUseHADME           );
  printf("SRD:%d ", m_bUseSBACRD          );
  printf("RDQ:%d ", m_bUseRDOQ            );
  printf("PAD:%d ", m_bUsePAD             );
#if !G1002_RPS
  printf("NRF:%d ", m_bUseNRF             );
  printf("BQP:%d ", m_bUseBQP             );
  printf("GPB:%d ", m_bUseGPB             );
#endif
  printf("FEN:%d ", m_bUseFastEnc         );
#if EARLY_CU_DETERMINATION
  printf("ECU:%d ", m_bUseEarlyCU         );
#endif
#if CBF_FAST_MODE
  printf("CFM:%d ", m_bUseCbfFastMode         );
#endif
  printf("RQT:%d ", 1     );
  printf("MRG:%d ", m_bUseMRG             ); // SOPH: Merge Mode
  printf("LMC:%d ", m_bUseLMChroma        ); 
  printf("CIP:%d ", m_bUseConstrainedIntraPred);
#if !G1002_RPS
#if REF_SETTING_FOR_LD
  printf("NewRefSetting:%d ", m_bUseNewRefSetting?1:0);
#endif
#endif

  printf("\n\n");
  
  fflush(stdout);
}

Void TAppEncCfg::xPrintUsage()
{
  printf( "          <name> = IBD - bit-depth increasement\n");
  printf( "                   GPB - generalized B instead of P in low-delay mode\n");
  printf( "                   HAD - hadamard ME for fractional-pel\n");
  printf( "                   SRD - SBAC based RD estimation\n");
  printf( "                   RDQ - RDOQ\n");
  printf( "                   LDC - low-delay mode\n");
  printf( "                   NRF - non-reference frame marking in last layer\n");
  printf( "                   BQP - hier-P style QP assignment in low-delay mode\n");
  printf( "                   PAD - automatic source padding of multiple of 16\n");
  printf( "                   ASR - adaptive motion search range\n");
  printf( "                   FEN - fast encoder setting\n");  
#if EARLY_CU_DETERMINATION
  printf( "                   ECU - Early CU setting\n");
#endif
#if CBF_FAST_MODE
  printf( "                   CFM - Cbf fast mode setting\n");
#endif
  printf( "                   MRG - merging of motion partitions\n"); // SOPH: Merge Mode

  printf( "                   LMC - intra chroma prediction based on luma\n");

  printf( "\n" );
  printf( "  Example 1) TAppEncoder.exe -c test.cfg -q 32 -g 8 -f 9 -s 64 -h 4\n");
  printf("              -> QP 32, hierarchical-B GOP 8, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
  printf( "  Example 2) TAppEncoder.exe -c test.cfg -q 32 -g 4 -f 9 -s 64 -h 4 -1 LDC\n");
  printf("              -> QP 32, hierarchical-P GOP 4, 9 frames, 64x64-8x8 CU (~4x4 PU)\n\n");
}

Bool confirmPara(Bool bflag, const char* message)
{
  if (!bflag)
    return false;
  
  printf("Error: %s\n",message);
  return true;
}

/* helper function */
/* for handling "-1/-0 FOO" */
void translateOldStyleCmdline(const char* value, po::Options& opts, const std::string& arg)
{
  const char* argv[] = {arg.c_str(), value};
  /* replace some short names with their long name varients */
  if (arg == "LDC")
  {
    argv[0] = "LowDelayCoding";
  }
  else if (arg == "RDQ")
  {
    argv[0] = "RDOQ";
  }
  else if (arg == "HAD")
  {
    argv[0] = "HadamardME";
  }
  else if (arg == "SRD")
  {
    argv[0] = "SBACRD";
  }
  else if (arg == "IBD")
  {
    argv[0] = "BitIncrement";
  }
  /* issue a warning for change in FEN behaviour */
  if (arg == "FEN")
  {
    /* xxx todo */
  }
  po::storePair(opts, argv[0], argv[1]);
}

void doOldStyleCmdlineOn(po::Options& opts, const std::string& arg)
{
  if (arg == "IBD")
  {
    translateOldStyleCmdline("4", opts, arg);
    return;
  }
  translateOldStyleCmdline("1", opts, arg);
}

void doOldStyleCmdlineOff(po::Options& opts, const std::string& arg)
{
  translateOldStyleCmdline("0", opts, arg);
}

//! \}
