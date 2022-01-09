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
"import random\n"
"import math\n"
"\n"
"def randommonotonic(start=0,sign = 1,n=10):\n"
"  answer = [start]\n"
"  for i in range(n-1):\n"
"    answer.append(answer[-1]+random.randint(0,2)*sign)\n"
"  return answer\n"
"\n"
"def randomint(n=10):\n"
"  answer = []\n"
"  for i in range(n):\n"
"    answer.append(random.randint(-2,2))\n"
"  return answer\n"
"  \n"
"def randomsequence(start=0,n=10):\n"
"  answer = [start]\n"
"  for i in range(n-1):\n"
"    answer.append(answer[-1]+random.normalvariate(0,2))\n"
"  return answer\n"
"  \n"
"def whitenoise(N): \n"
"   data = []\n"
"   for i in range(N):\n"
"                data.append(random.normalvariate(0.0,1.0))\n"
"   return data\n"
"\n"
"\n"
"def randomwalk(N): \n"
"   value = 0.0\n"
"   data = []\n"
"   for i in range(N):\n"
"                data.append(value)\n"
"                value += random.normalvariate(0.0,1.0)\n"
"   return data\n"
"\n"
"\n"
"\n"
"#########################\n"
"# Control Charts\n"
"# @article{pham1998ccp,\r#  title={{Control chart pattern recognition using a new type of self-organizing neural network}},\r#  author={Pham, D.T. and Chan, A.B.},\r#  journal={Proceedings of the Institution of Mechanical Engineers, Part I: Journal o"
"f Systems and Control Engineering},\r#  volume={212},\r#  number={2},\r#  pages={115--127},\r#  year={1998},\r#  publisher={Prof Eng Publishing}\r#}\n"
"######################\n"
"\n"
"NORMAL = 1\n"
"CYCLIC = 2\n"
"INCREASING = 3\n"
"DECREASING = 4\n"
"UPWARD = 5\n"
"DOWNWARD = 6\n"
"\n"
"def controlcharts(o,n = 60):\n"
"  m = 30\n"
"  s = 2\n"
"  if(o == NORMAL): \n"
"    return [m+random.uniform(-3,3)*s for t in range(1,n+1)]\n"
"  if(o ==  CYCLIC):\n"
"    a = random.uniform(10,15)\n"
"    T = random.uniform(10,15)\n"
"    return [m+random.uniform(-3,3)*s+a*math.sin(2*math.pi*t/T) for t in range(1,n+1)]\n"
"  if(o ==  INCREASING):\n"
"    g= random.uniform(0.2,0.5)\n"
"    return [m+random.uniform(-3,3)*s+g*t for t in range(1,n+1)]\n"
"  if( o == DECREASING) :\n"
"    g= random.uniform(0.2,0.5)\n"
"    return [m+random.uniform(-3,3)*s-g*t for t in range(1,n+1)]\n"
"  if( o == UPWARD) :\n"
"    x = random.uniform(7.5,20)\n"
"    assert(n/3 * 3 == n)\n"
"    t3 = random.uniform(n/3,2*n/3)\n"
"    def k(t): \n"
"      if(t< t3): return 0\n"
"      return 1\n"
"    return [m+random.uniform(-3,3)*s+k(t)*x for t in range(1,n+1)]\n"
"  if( o == DOWNWARD) :\n"
"    x = random.uniform(7.5,20)\n"
"    assert(n/3 * 3 == n)\n"
"    t3 = random.uniform(n/3,2*n/3)\n"
"    def k(t): \n"
"      if(t< t3): return 0\n"
"      return 1\n"
"    return [m+random.uniform(-3,3)*s-k(t)*x for t in range(1,n+1)]\n"
"  return None\n"
"\n"
"#######################\n"
"# Waveform \n"
"# L. Breiman, J.H. Friedman, A. Olshen, and \n"
"# C.J. Stone. Classification and Regression Trees. \n"
"#Chapman & Hall, New York, 1993. Previ- \n"
"#ously published by Wadsworth & Brooks/Cole \n"
"#in 1984. \n"
"def __h1(i): return max(6-abs(i-7),0)\n"
"def __h2(i): return __h1(i-8)\n"
"def __h3(i): return __h1(i-4)\n"
"\n"
"def wave(o):\n"
"  u = random.uniform(0,1)\n"
"  if(o == 1):\n"
"    return [u*__h1(i)+(1-u)*__h2(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\n"
"  if(o == 2):\n"
"    return [u*__h1(i)+(1-u)*__h3(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\n"
"  if(o == 3):\n"
"    return [u*__h2(i)+(1-u)*__h3(i)+random.normalvariate(0.0,1.0) for i in range(1,22)]\n"
"  return None\n"
"\n"
"#######################\n"
"# Wave+Noise\n"
"#@article{gonzalez2000tsc,\r#  title={{Time Series Classification by Boosting Interval Based Literals}},\r#  author={Gonzalez, C.A. and Diez, J.J.R.},\r#  journal={Inteligencia Artificial, Revista Iberoamericana de Inteligencia Artificial},\r#  volum"
"e={11},\r#  pages={2--11},\r#  year={2000}\r#}\n"
"\n"
"def waveplusnoise(o):\n"
"  return wave(o)+[random.normalvariate(0.0,1.0) for i in range(19)]\n"
"  \n"
"\n"
"######################\n"
"# CBF data set (Cylinder, Bell and Funnel)\n"
"# Naoki Saito. Local Feature Extraction and Its \n"
"# Applications Using a Library of Bases. PhD \n"
"# thesis, Department of Mathematics, Yale Uni- \n"
"# versity, 1994. \n"
"######################\n"
"\n"
"CYLINDER=1\n"
"BELL=2\n"
"FUNNEL=3\n"
"\n"
"def cbf(o):\n"
"  \"\"\"\n"
"  This is the Cylinder-Bell-Funnel (CBF)\n"
"   N. Saito, Local feature extraction and its application\n"
"  using a library of bases. Ph.D. thesis, Department of Mathematics,\n"
"  Yale University, 1994.\"\"\"\n"
"  a = random.randint(16,32)\n"
"  b = random.randint(32,96) + a\n"
"  n = random.normalvariate(0.0,1.0)\n"
"  def xab(t):\n"
"    if ( (t>=a) and (t <=b) ):\n"
"      return 1.0\n"
"    return 0.0\n"
"  if(o == CYLINDER):\n"
"    return [ (6+n)*xab(t) + random.normalvariate(0.0,1.0) for t in range(1,129)]\n"
"  elif(o == BELL) :\n"
"    return [ (6+n)*xab(t) *(t - a) / (b - a)  + random.normalvariate(0.0,1.0) for t in range(1,129)]    \n"
"  elif (o == FUNNEL):\n"
"    return [ (6+n)*xab(t) *(b - t) / (b - a)  + random.normalvariate(0.0,1.0) for t in range(1,129)]    \n"
"  else :\n"
"    return None\n"
"   \n"
"def plotcbf():\n"
"  \"This requires the Gnuplot package\" \n"
"  import Gnuplot\n"
"  g = Gnuplot.Gnuplot(debug=1)\n"
"  c=cbf(CYLINDER)\n"
"  b=cbf(BELL)\n"
"  f=cbf(FUNNEL) \n"
"  t1 = Gnuplot.Data([[i,c[i]] for i in range(128)],with=\"lines lw 5\",title=\"cylinder\")\n"
"  t2 = Gnuplot.Data([[i,b[i]] for i in range(128)],with=\"lines lw 5\",title=\"bell\")\n"
"  t3 = Gnuplot.Data([[i,f[i]] for i in range(128)],with=\"lines lw 5\",title=\"funnel\")\n"
"  g.plot(t1,t2,t3)\n"
"\n"
"if __name__==\"__main__\":\n"
"  plotcbf()\n"
"\n"
"######################################## \n";

const char* syntheticdata_py = (const char*) temp_binary_data_7;

//================== LICENSE.txt ==================
static const unsigned char temp_binary_data_8[] =
"                                 Apache License\n"
"                           Version 2.0, January 2004\n"
"                        http://www.apache.org/licenses/\n"
"\n"
"   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION\n"
"\n"
"   1. Definitions.\n"
"\n"
"      \"License\" shall mean the terms and conditions for use, reproduction,\n"
"      and distribution as defined by Sections 1 through 9 of this document.\n"
"\n"
"      \"Licensor\" shall mean the copyright owner or entity authorized by\n"
"      the copyright owner that is granting the License.\n"
"\n"
"      \"Legal Entity\" shall mean the union of the acting entity and all\n"
"      other entities that control, are controlled by, or are under common\n"
"      control with that entity. For the purposes of this definition,\n"
"      \"control\" means (i) the power, direct or indirect, to cause the\n"
"      direction or management of such entity, whether by contract or\n"
"      otherwise, or (ii) ownership of fifty percent (50%) or more of the\n"
"      outstanding shares, or (iii) beneficial ownership of such entity.\n"
"\n"
"      \"You\" (or \"Your\") shall mean an individual or Legal Entity\n"
"      exercising permissions granted by this License.\n"
"\n"
"      \"Source\" form shall mean the preferred form for making modifications,\n"
"      including but not limited to software source code, documentation\n"
"      source, and configuration files.\n"
"\n"
"      \"Object\" form shall mean any form resulting from mechanical\n"
"      transformation or translation of a Source form, including but\n"
"      not limited to compiled object code, generated documentation,\n"
"      and conversions to other media types.\n"
"\n"
"      \"Work\" shall mean the work of authorship, whether in Source or\n"
"      Object form, made available under the License, as indicated by a\n"
"      copyright notice that is included in or attached to the work\n"
"      (an example is provided in the Appendix below).\n"
"\n"
"      \"Derivative Works\" shall mean any work, whether in Source or Object\n"
"      form, that is based on (or derived from) the Work and for which the\n"
"      editorial revisions, annotations, elaborations, or other modifications\n"
"      represent, as a whole, an original work of authorship. For the purposes\n"
"      of this License, Derivative Works shall not include works that remain\n"
"      separable from, or merely link (or bind by name) to the interfaces of,\n"
"      the Work and Derivative Works thereof.\n"
"\n"
"      \"Contribution\" shall mean any work of authorship, including\n"
"      the original version of the Work and any modifications or additions\n"
"      to that Work or Derivative Works thereof, that is intentionally\n"
"      submitted to Licensor for inclusion in the Work by the copyright owner\n"
"      or by an individual or Legal Entity authorized to submit on behalf of\n"
"      the copyright owner. For the purposes of this definition, \"submitted\"\n"
"      means any form of electronic, verbal, or written communication sent\n"
"      to the Licensor or its representatives, including but not limited to\n"
"      communication on electronic mailing lists, source code control systems,\n"
"      and issue tracking systems that are managed by, or on behalf of, the\n"
"      Licensor for the purpose of discussing and improving the Work, but\n"
"      excluding communication that is conspicuously marked or otherwise\n"
"      designated in writing by the copyright owner as \"Not a Contribution.\"\n"
"\n"
"      \"Contributor\" shall mean Licensor and any individual or Legal Entity\n"
"      on behalf of whom a Contribution has been received by Licensor and\n"
"      subsequently incorporated within the Work.\n"
"\n"
"   2. Grant of Copyright License. Subject to the terms and conditions of\n"
"      this License, each Contributor hereby grants to You a perpetual,\n"
"      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n"
"      copyright license to reproduce, prepare Derivative Works of,\n"
"      publicly display, publicly perform, sublicense, and distribute the\n"
"      Work and such Derivative Works in Source or Object form.\n"
"\n"
"   3. Grant of Patent License. Subject to the terms and conditions of\n"
"      this License, each Contributor hereby grants to You a perpetual,\n"
"      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n"
"      (except as stated in this section) patent license to make, have made,\n"
"      use, offer to sell, sell, import, and otherwise transfer the Work,\n"
"      where such license applies only to those patent claims licensable\n"
"      by such Contributor that are necessarily infringed by their\n"
"      Contribution(s) alone or by combination of their Contribution(s)\n"
"      with the Work to which such Contribution(s) was submitted. If You\n"
"      institute patent litigation against any entity (including a\n"
"      cross-claim or counterclaim in a lawsuit) alleging that the Work\n"
"      or a Contribution incorporated within the Work constitutes direct\n"
"      or contributory patent infringement, then any patent licenses\n"
"      granted to You under this License for that Work shall terminate\n"
"      as of the date such litigation is filed.\n"
"\n"
"   4. Redistribution. You may reproduce and distribute copies of the\n"
"      Work or Derivative Works thereof in any medium, with or without\n"
"      modifications, and in Source or Object form, provided that You\n"
"      meet the following conditions:\n"
"\n"
"      (a) You must give any other recipients of the Work or\n"
"          Derivative Works a copy of this License; and\n"
"\n"
"      (b) You must cause any modified files to carry prominent notices\n"
"          stating that You changed the files; and\n"
"\n"
"      (c) You must retain, in the Source form of any Derivative Works\n"
"          that You distribute, all copyright, patent, trademark, and\n"
"          attribution notices from the Source form of the Work,\n"
"          excluding those notices that do not pertain to any part of\n"
"          the Derivative Works; and\n"
"\n"
"      (d) If the Work includes a \"NOTICE\" text file as part of its\n"
"          distribution, then any Derivative Works that You distribute must\n"
"          include a readable copy of the attribution notices contained\n"
"          within such NOTICE file, excluding those notices that do not\n"
"          pertain to any part of the Derivative Works, in at least one\n"
"          of the following places: within a NOTICE text file distributed\n"
"          as part of the Derivative Works; within the Source form or\n"
"          documentation, if provided along with the Derivative Works; or,\n"
"          within a display generated by the Derivative Works, if and\n"
"          wherever such third-party notices normally appear. The contents\n"
"          of the NOTICE file are for informational purposes only and\n"
"          do not modify the License. You may add Your own attribution\n"
"          notices within Derivative Works that You distribute, alongside\n"
"          or as an addendum to the NOTICE text from the Work, provided\n"
"          that such additional attribution notices cannot be construed\n"
"          as modifying the License.\n"
"\n"
"      You may add Your own copyright statement to Your modifications and\n"
"      may provide additional or different license terms and conditions\n"
"      for use, reproduction, or distribution of Your modifications, or\n"
"      for any such Derivative Works as a whole, provided Your use,\n"
"      reproduction, and distribution of the Work otherwise complies with\n"
"      the conditions stated in this License.\n"
"\n"
"   5. Submission of Contributions. Unless You explicitly state otherwise,\n"
"      any Contribution intentionally submitted for inclusion in the Work\n"
"      by You to the Licensor shall be under the terms and conditions of\n"
"      this License, without any additional terms or conditions.\n"
"      Notwithstanding the above, nothing herein shall supersede or modify\n"
"      the terms of any separate license agreement you may have executed\n"
"      with Licensor regarding such Contributions.\n"
"\n"
"   6. Trademarks. This License does not grant permission to use the trade\n"
"      names, trademarks, service marks, or product names of the Licensor,\n"
"      except as required for reasonable and customary use in describing the\n"
"      origin of the Work and reproducing the content of the NOTICE file.\n"
"\n"
"   7. Disclaimer of Warranty. Unless required by applicable law or\n"
"      agreed to in writing, Licensor provides the Work (and each\n"
"      Contributor provides its Contributions) on an \"AS IS\" BASIS,\n"
"      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or\n"
"      implied, including, without limitation, any warranties or conditions\n"
"      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A\n"
"      PARTICULAR PURPOSE. You are solely responsible for determining the\n"
"      appropriateness of using or redistributing the Work and assume any\n"
"      risks associated with Your exercise of permissions under this License.\n"
"\n"
"   8. Limitation of Liability. In no event and under no legal theory,\n"
"      whether in tort (including negligence), contract, or otherwise,\n"
"      unless required by applicable law (such as deliberate and grossly\n"
"      negligent acts) or agreed to in writing, shall any Contributor be\n"
"      liable to You for damages, including any direct, indirect, special,\n"
"      incidental, or consequential damages of any character arising as a\n"
"      result of this License or out of the use or inability to use the\n"
"      Work (including but not limited to damages for loss of goodwill,\n"
"      work stoppage, computer failure or malfunction, or any and all\n"
"      other commercial damages or losses), even if such Contributor\n"
"      has been advised of the possibility of such damages.\n"
"\n"
"   9. Accepting Warranty or Additional Liability. While redistributing\n"
"      the Work or Derivative Works thereof, You may choose to offer,\n"
"      and charge a fee for, acceptance of support, warranty, indemnity,\n"
"      or other liability obligations and/or rights consistent with this\n"
"      License. However, in accepting such obligations, You may act only\n"
"      on Your own behalf and on Your sole responsibility, not on behalf\n"
"      of any other Contributor, and only if You agree to indemnify,\n"
"      defend, and hold each Contributor harmless for any liability\n"
"      incurred by, or claims asserted against, such Contributor by reason\n"
"      of your accepting any such warranty or additional liability.\n"
"\n"
"   END OF TERMS AND CONDITIONS\n"
"\n"
"   APPENDIX: How to apply the Apache License to your work.\n"
"\n"
"      To apply the Apache License to your work, attach the following\n"
"      boilerplate notice, with the fields enclosed by brackets \"{}\"\n"
"      replaced with your own identifying information. (Don't include\n"
"      the brackets!)  The text should be enclosed in the appropriate\n"
"      comment syntax for the file format. We also recommend that a\n"
"      file or class name and description of purpose be included on the\n"
"      same \"printed page\" as the copyright notice for easier\n"
"      identification within third-party archives.\n"
"\n"
"   Copyright {yyyy} {name of copyright owner}\n"
"\n"
"   Licensed under the Apache License, Version 2.0 (the \"License\");\n"
"   you may not use this file except in compliance with the License.\n"
"   You may obtain a copy of the License at\n"
"\n"
"       http://www.apache.org/licenses/LICENSE-2.0\n"
"\n"
"   Unless required by applicable law or agreed to in writing, software\n"
"   distributed under the License is distributed on an \"AS IS\" BASIS,\n"
"   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
"   See the License for the specific language governing permissions and\n"
"   limitations under the License.\n";

const char* LICENSE_txt = (const char*) temp_binary_data_8;

//================== Makefile ==================
static const unsigned char temp_binary_data_9[] =
"all: unittesting benchmark example\n"
"OSNAME = $(shell uname | tr \"[:upper:]\" \"[:lower:]\")\n"
"SHAREDNAME=$(shell if [  $(OSNAME) = \"darwin\" ]; then echo -n \"   -bundle -flat_namespace -undefined suppress\"; else echo -n \"-shared\";fi )\n"
"\n"
"\n"
"benchmark: benchmarks/benchmark.cpp include/dtw.h \n"
"\t$(CXX) -O2 -Wall -Wold-style-cast  -Woverloaded-virtual -o benchmark benchmarks/benchmark.cpp  -Iinclude\n"
"\n"
"example: examples/example.cpp include/dtw.h \n"
"\t$(CXX) -O2 -Wall -Wold-style-cast  -Woverloaded-virtual -o example examples/example.cpp  -Iinclude\n"
"\n"
"\n"
"unittesting: tests/unittesting.cpp include/dtw.h \n"
"\t$(CXX) -g3 -Wall -Wold-style-cast  -Woverloaded-virtual -o unittesting tests/unittesting.cpp -Iinclude\n"
"\n"
"\n"
"clean :\n"
"\trm -f benchmark unittesting example\n";

const char* Makefile = (const char*) temp_binary_data_9;

//================== README.md ==================
static const unsigned char temp_binary_data_10[] =
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
"Comments about this paper by Keogh's team: \n"
"\n"
"     To our knowledge, there is only one paper that\n"
"     offers a plausible speedup based on a tighter \n"
"     lower bound\xe2\x80\x94Lemire (2009) suggests a mean speedup \n"
"     of about 1.4 based on a tighter bound. \n"
"     These results are reproducible, and testing on \n"
"     more general data sets we obtained similar \n"
"     results (...) (Wang et al. 2013)\n"
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
"\n"
"# Other libraries\n"
" \n"
"*  [dtwclust](https://github.com/asardaes/dtwclust) is an  R Package for Time Series Clustering Along with Optimizations for DTW\n";

const char* README_md = (const char*) temp_binary_data_10;


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
        case 0xcfbfbbd9:  numBytes = 4978; return syntheticdata_py;
        case 0x5a320952:  numBytes = 11357; return LICENSE_txt;
        case 0x064cb88a:  numBytes = 736; return Makefile;
        case 0x64791dc8:  numBytes = 1247; return README_md;
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
    "LICENSE_txt",
    "Makefile",
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
    "LICENSE.txt",
    "Makefile",
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
