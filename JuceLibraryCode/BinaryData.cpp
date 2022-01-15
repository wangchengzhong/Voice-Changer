/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== Makefile.am ==================
static const unsigned char temp_binary_data_0[] =
"## Process this file with automake to create Makefile.in\r\n"
"##\r\n"
"## This file is part of SoundTouch, an audio processing library for pitch/time adjustments\r\n"
"## \r\n"
"## SoundTouch is free software; you can redistribute it and/or modify it under the\r\n"
"## terms of the GNU General Public License as published by the Free Software\r\n"
"## Foundation; either version 2 of the License, or (at your option) any later\r\n"
"## version.\r\n"
"## \r\n"
"## SoundTouch is distributed in the hope that it will be useful, but WITHOUT ANY\r\n"
"## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR\r\n"
"## A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\r\n"
"## \r\n"
"## You should have received a copy of the GNU General Public License along with\r\n"
"## this program; if not, write to the Free Software Foundation, Inc., 59 Temple\r\n"
"## Place - Suite 330, Boston, MA  02111-1307, USA\r\n"
"\r\n"
"## I used config/am_include.mk for common definitions\r\n"
"include $(top_srcdir)/config/am_include.mk\r\n"
"\r\n"
"pkginclude_HEADERS=FIFOSampleBuffer.h FIFOSamplePipe.h SoundTouch.h STTypes.h BPMDetect.h soundtouch_config.h\r\n"
"\r\n";

const char* Makefile_am = (const char*) temp_binary_data_0;

//================== soundtouch_config.h.in ==================
static const unsigned char temp_binary_data_1[] =
"/* Use Float as Sample type */\n"
"#undef SOUNDTOUCH_FLOAT_SAMPLES\n"
"\n"
"/* Use Integer as Sample type */\n"
"#undef SOUNDTOUCH_INTEGER_SAMPLES\n"
"\n"
"/* Use ARM NEON extension */\n"
"#undef SOUNDTOUCH_USE_NEON\n";

const char* soundtouch_config_h_in = (const char*) temp_binary_data_1;

//================== COPYING ==================
static const unsigned char temp_binary_data_2[] =
"Copyright (c) 2003-2010 Mark Borgerding . All rights reserved.\n"
"\n"
"KISS FFT is provided under:\n"
"\n"
"  SPDX-License-Identifier: BSD-3-Clause\n"
"\n"
"Being under the terms of the BSD 3-clause \"New\" or \"Revised\" License,\n"
"according with:\n"
"\n"
"  LICENSES/BSD-3-Clause\n"
"\n";

const char* COPYING = (const char*) temp_binary_data_2;

//================== COPYING ==================
static const unsigned char temp_binary_data_3[] =
"Copyright 2002-2007 \tXiph.org Foundation\n"
"Copyright 2002-2007 \tJean-Marc Valin\n"
"Copyright 2005-2007\tAnalog Devices Inc.\n"
"Copyright 2005-2007\tCommonwealth Scientific and Industrial Research \n"
"                        Organisation (CSIRO)\n"
"Copyright 1993, 2002, 2006 David Rowe\n"
"Copyright 2003 \t\tEpicGames\n"
"Copyright 1992-1994\tJutta Degener, Carsten Bormann\n"
"\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"\n"
"- Redistributions of source code must retain the above copyright\n"
"notice, this list of conditions and the following disclaimer.\n"
"\n"
"- Redistributions in binary form must reproduce the above copyright\n"
"notice, this list of conditions and the following disclaimer in the\n"
"documentation and/or other materials provided with the distribution.\n"
"\n"
"- Neither the name of the Xiph.org Foundation nor the names of its\n"
"contributors may be used to endorse or promote products derived from\n"
"this software without specific prior written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR\n"
"CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";

const char* COPYING2 = (const char*) temp_binary_data_3;

//================== __init__.py ==================
static const unsigned char temp_binary_data_4[] =
"";

const char* __init___py = (const char*) temp_binary_data_4;

//================== flatfileparser.py ==================
static const unsigned char temp_binary_data_5[] =
"\n"
"\n"
"def readFlat(filename, delimiter):\n"
"  f = open(filename)\n"
"  ans = []\n"
"  for line in f:\n"
"   ans.append(map(lambda x:float(x), filter(lambda x:len(x)>0,line.split(delimiter))))\n"
"  return ans\n"
"\n";

const char* flatfileparser_py = (const char*) temp_binary_data_5;

//================== stockmarketparser.py ==================
static const unsigned char temp_binary_data_6[] =
"import re\n"
"import time\n"
"\n"
"def readyahoofinance(filename):\n"
"  \"\"\" as the name suggests\"\"\"\n"
"  regex = \"([0-9]+-[A-Za-z]+-[0-9]+),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]+)\"\n"
"  # date format seems to be month/day/year\n"
"  data = []\n"
"  in_file =open(filename,'r')\n"
"  for line in in_file:\n"
"    #print line\n"
"    matchedcontent = re.match(regex,line)\n"
"    #print matchedcontent\n"
"    if(matchedcontent<> None):\n"
"      #print matchedcontent.group(1)\n"
"      try:\n"
"        date =round(time.mktime( time.strptime(matchedcontent.group(1),\"%d-%b-%y\"))/(3600*24)) #days since epoch\n"
"      except OverflowError:\n"
"        continue\n"
"      #print date\n"
"      openval = float(matchedcontent.group(2))\n"
"      #print open\n"
"      high = float(matchedcontent.group(3))\n"
"      #print high\n"
"      low = float(matchedcontent.group(4))\n"
"      #print low\n"
"      close = float(matchedcontent.group(5))\n"
"      #print close\n"
"      volume = int(matchedcontent.group(6))\n"
"      #print volume\n"
"      data.append((date,low))#close))\n"
"  print \"read \", len(data), \" data points!\"\n"
"  in_file.close()\n"
"  assert len(data)>0\n"
"  data.sort()\n"
"  #data.reverse()\n"
"  return data\n"
"\n"
"def readmoneycentral(filename):\n"
"  \"\"\" This reads data from Microsoft Money Central web site\"\"\"\n"
"  regex = \"([0-9]+/[0-9]+/[0-9]+),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]*\\.[0-9]*),([0-9]+)\"\n"
"  # date format seems to be month/day/year\n"
"  data = []\n"
"  in_file =open(filename)\n"
"  for line in in_file:\n"
"    #print line\n"
"    matchedcontent = re.match(regex,line)\n"
"    #print matchedcontent\n"
"    if(matchedcontent<> None):\n"
"      date =round(time.mktime( time.strptime(matchedcontent.group(1),\"%m/%d/%Y\"))/(3600*24)) #days since epoch\n"
"      #print date\n"
"      openval = float(matchedcontent.group(2))\n"
"      #print open\n"
"      high = float(matchedcontent.group(3))\n"
"      #print high\n"
"      low = float(matchedcontent.group(4))\n"
"      #print low\n"
"      close = float(matchedcontent.group(5))\n"
"      #print close\n"
"      volume = int(matchedcontent.group(6))\n"
"      #print volume\n"
"      data.append((date,low))#close))\n"
"  print \"read \", len(data), \" data points!\"\n"
"  in_file.close()\n"
"  assert len(data)>0\n"
"  data.sort()\n"
"  #data.reverse()\n"
"  return data\n"
"\n";

const char* stockmarketparser_py = (const char*) temp_binary_data_6;

//================== syntheticdata.py ==================
static const unsigned char temp_binary_data_7[] =
"import random\r\n"
"import math\r\n"
"\r\n"
"def randommonotonic(start=0,sign = 1,n=10):\r\n"
"  answer = [start]\r\n"
"  for i in range(n-1):\r\n"
"    answer.append(answer[-1]+random.randint(0,2)*sign)\r\n"
"  return answer\r\n"
"\r\n"
"def randomint(n=10):\r\n"
"  answer = []\r\n"
"  for i in range(n):\r\n"
"    answer.append(random.randint(-2,2))\r\n"
"  return answer\r\n"
"  \r\n"
"def randomsequence(start=0,n=10):\r\n"
"  answer = [start]\r\n"
"  for i in range(n-1):\r\n"
"    answer.append(answer[-1]+random.normalvariate(0,2))\r\n"
"  return answer\r\n"
"  \r\n"
"def whitenoise(N): \r\n"
"   data = []\r\n"
"   for i in range(N):\r\n"
"                data.append(random.normalvariate(0.0,1.0))\r\n"
"   return data\r\n"
"\r\n"
"\r\n"
"def randomwalk(N): \r\n"
"   value = 0.0\r\n"
"   data = []\r\n"
"   for i in range(N):\r\n"
"                data.append(value)\r\n"
"                value += random.normalvariate(0.0,1.0)\r\n"
"   return data\r\n"
"\r\n"
"\r\n"
"\r\n"
"#########################\r\n"
"# Control Charts\r\n"
"# @article{pham1998ccp,\r\n"
"#  title={{Control chart pattern recognition using a new type of self-organizing neural network}},\r\n"
"#  author={Pham, D.T. and Chan, A.B.},\r\n"
"#  journal={Proceedings of the Institution of Mechanical Engineers, Part I: Journal of Systems and Control Engineering},\r\n"
"#  volume={212},\r\n"
"#  number={2},\r\n"
"#  pages={115--127},\r\n"
"#  year={1998},\r\n"
"#  publisher={Prof Eng Publishing}\r\n"
"#}\r\n"
"######################\r\n"
"\r\n"
"NORMAL = 1\r\n"
"CYCLIC = 2\r\n"
"INCREASING = 3\r\n"
"DECREASING = 4\r\n"
"UPWARD = 5\r\n"
"DOWNWARD = 6\r\n"
"\r\n"
"def controlcharts(o,n = 60):\r\n"
"  m = 30\r\n"
"  s = 2\r\n"
"  if(o == NORMAL): \r\n"
"    return [m+random.uniform(-3,3)*s for t in range(1,n+1)]\r\n"
"  if(o ==  CYCLIC):\r\n"
"    a = random.uniform(10,15)\r\n"
"    T = random.uniform(10,15)\r\n"
"    return [m+random.uniform(-3,3)*s+a*math.sin(2*math.pi*t/T) for t in range(1,n+1)]\r\n"
"  if(o ==  INCREASING):\r\n"
"    g= random.uniform(0.2,0.5)\r\n"
"    return [m+random.uniform(-3,3)*s+g*t for t in range(1,n+1)]\r\n"
"  if( o == DECREASING) :\r\n"
"    g= random.uniform(0.2,0.5)\r\n"
"    return [m+random.uniform(-3,3)*s-g*t for t in range(1,n+1)]\r\n"
"  if( o == UPWARD) :\r\n"
"    x = random.uniform(7.5,20)\r\n"
"    assert(n/3 * 3 == n)\r\n"
"    t3 = random.uniform(n/3,2*n/3)\r\n"
"    def k(t): \r\n"
"      if(t< t3): return 0\r\n"
"      return 1\r\n"
"    return [m+random.uniform(-3,3)*s+k(t)*x for t in range(1,n+1)]\r\n"
"  if( o == DOWNWARD) :\r\n"
"    x = random.uniform(7.5,20)\r\n"
"    assert(n/3 * 3 == n)\r\n"
"    t3 = random.uniform(n/3,2*n/3)\r\n"
"    def k(t): \r\n"
"      if(t< t3): return 0\r\n"
"      return 1\r\n"
"    return [m+random.uniform(-3,3)*s-k(t)*x for t in range(1,n+1)]\r\n"
"  return None\r\n"
"\r\n"
"#######################\r\n"
"# Waveform \r\n"
"# L. Breiman, J.H. Friedman, A. Olshen, and \r\n"
"# C.J. Stone. Classification and Regression Trees. \r\n"
"#Chapman & Hall, New York, 1993. Previ- \r\n"
"#ously published by Wadsworth & Brooks/Cole \r\n"
"#in 1984. \r\n"
"def __h1(i): return max(6-abs(i-7),0)\r\n"
"def __h2(i): return __h1(i-8)\r\n"
"def __h3(i): return __h1(i-4)\r\n"
"\r\n"
"def wave(o):\r\n"
"  u = random.uniform(0,1)\r\n"
"  if(o == 1):\r\n"
"    return [u*__h1(i)+(1-u)*__h2(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\r\n"
"  if(o == 2):\r\n"
"    return [u*__h1(i)+(1-u)*__h3(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\r\n"
"  if(o == 3):\r\n"
"    return [u*__h2(i)+(1-u)*__h3(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\r\n"
"  return None\r\n"
"\r\n"
"#######################\r\n"
"# Wave+Noise\r\n"
"#@article{gonzalez2000tsc,\r\n"
"#  title={{Time Series Classification by Boosting Interval Based Literals}},\r\n"
"#  author={Gonzalez, C.A. and Diez, J.J.R.},\r\n"
"#  journal={Inteligencia Artificial, Revista Iberoamericana de Inteligencia Artificial},\r\n"
"#  volume={11},\r\n"
"#  pages={2--11},\r\n"
"#  year={2000}\r\n"
"#}\r\n"
"\r\n"
"def waveplusnoise(o):\r\n"
"  return wave(o)+[random.normalvariate(0.0,1.0) for i in range(19)]\r\n"
"  \r\n"
"\r\n"
"######################\r\n"
"# CBF data set (Cylinder, Bell and Funnel)\r\n"
"# Naoki Saito. Local Feature Extraction and Its \r\n"
"# Applications Using a Library of Bases. PhD \r\n"
"# thesis, Department of Mathematics, Yale Uni- \r\n"
"# versity, 1994. \r\n"
"######################\r\n"
"\r\n"
"CYLINDER=1\r\n"
"BELL=2\r\n"
"FUNNEL=3\r\n"
"\r\n"
"def cbf(o):\r\n"
"  \"\"\"\r\n"
"  This is the Cylinder-Bell-Funnel (CBF)\r\n"
"   N. Saito, Local feature extraction and its application\r\n"
"  using a library of bases. Ph.D. thesis, Department of Mathematics,\r\n"
"  Yale University, 1994.\"\"\"\r\n"
"  a = random.randint(16,32)\r\n"
"  b = random.randint(32,96) + a\r\n"
"  n = random.normalvariate(0.0,1.0)\r\n"
"  def xab(t):\r\n"
"    if ( (t>=a) and (t <=b) ):\r\n"
"      return 1.0\r\n"
"    return 0.0\r\n"
"  if(o == CYLINDER):\r\n"
"    return [ (6+n)*xab(t) + random.normalvariate(0.0,1.0) for t in range(1,129)]\r\n"
"  elif(o == BELL) :\r\n"
"    return [ (6+n)*xab(t) *(t - a) / (b - a)  + random.normalvariate(0.0,1.0) for t in range(1,129)]    \r\n"
"  elif (o == FUNNEL):\r\n"
"    return [ (6+n)*xab(t) *(b - t) / (b - a)  + random.normalvariate(0.0,1.0) for t in range(1,129)]    \r\n"
"  else :\r\n"
"    return None\r\n"
"   \r\n"
"def plotcbf():\r\n"
"  \"This requires the Gnuplot package\" \r\n"
"  import Gnuplot\r\n"
"  g = Gnuplot.Gnuplot(debug=1)\r\n"
"  c=cbf(CYLINDER)\r\n"
"  b=cbf(BELL)\r\n"
"  f=cbf(FUNNEL) \r\n"
"  t1 = Gnuplot.Data([[i,c[i]] for i in range(128)],with=\"lines lw 5\",title=\"cylinder\")\r\n"
"  t2 = Gnuplot.Data([[i,b[i]] for i in range(128)],with=\"lines lw 5\",title=\"bell\")\r\n"
"  t3 = Gnuplot.Data([[i,f[i]] for i in range(128)],with=\"lines lw 5\",title=\"funnel\")\r\n"
"  g.plot(t1,t2,t3)\r\n"
"\r\n"
"if __name__==\"__main__\":\r\n"
"  plotcbf()\r\n"
"\r\n"
"######################################## \r\n";

const char* syntheticdata_py = (const char*) temp_binary_data_7;

//================== README.md ==================
static const unsigned char temp_binary_data_8[] =
"# LBImproved C++ Library\n"
"[![Build Status](https://travis-ci.org/lemire/lbimproved.png)](https://travis-ci.org/lemire/lbimproved)\n"
"\n"
"This library comes in the form of one short C++ header file. The documentation\n"
"is in the C++ comments and in this file.\n"
"\n"
"\n"
"# Key feature\n"
"\n"
"1) Fast Dynamic Time Warping nearest neighbor retrieval.\n"
"\n"
"2) Implementations of LB Koegh and LB Improved\n"
"\n"
"3) Companion to the following paper :\n"
"\n"
"Daniel Lemire, Faster Retrieval with a Two-Pass Dynamic-Time-Warping Lower Bound, Pattern Recognition 42 (9), pages 2169-2180, 2009. \n"
"http://arxiv.org/abs/0811.3301\n"
"\n"
"\n"
"\n"
"# BUILD \n"
"\n"
"type \"make\"\n"
"\n"
"    make\n"
"    ./unittesting\n"
"    ./benchmark\n"
"    ./example\n"
"\n"
"# Simple code example\n"
"\n"
"See ``example.cpp``.\n"
"\n";

const char* README_md = (const char*) temp_binary_data_8;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x1a187401:  numBytes = 1096; return Makefile_am;
        case 0x6fff950a:  numBytes = 188; return soundtouch_config_h_in;
        case 0x63a1442d:  numBytes = 246; return COPYING;
        case 0x108741a5:  numBytes = 1774; return COPYING2;
        case 0xd8b9efd8:  numBytes = 0; return __init___py;
        case 0xa62bf154:  numBytes = 187; return flatfileparser_py;
        case 0x3b2c7797:  numBytes = 2168; return stockmarketparser_py;
        case 0xcfbfbbd9:  numBytes = 5156; return syntheticdata_py;
        case 0x64791dc8:  numBytes = 706; return README_md;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "Makefile_am",
    "soundtouch_config_h_in",
    "COPYING",
    "COPYING2",
    "__init___py",
    "flatfileparser_py",
    "stockmarketparser_py",
    "syntheticdata_py",
    "README_md"
};

const char* originalFilenames[] =
{
    "Makefile.am",
    "soundtouch_config.h.in",
    "COPYING",
    "COPYING",
    "__init__.py",
    "flatfileparser.py",
    "stockmarketparser.py",
    "syntheticdata.py",
    "README.md"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}
