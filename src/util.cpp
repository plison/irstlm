// $Id: util.cpp 363 2010-02-22 15:02:45Z mfederico $
/******************************************************************************
IrstLM: IRST Language Model Toolkit
Copyright (C) 2006 Marcello Federico, ITC-irst Trento, Italy

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

******************************************************************************/

#ifdef WIN32
#include <windows.h>
#include <string.h>
#include <io.h>
#else
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/mman.h>
#endif

#include "timer.h"
#include "util.h"

using namespace std;

string gettempfolder()
{
#ifdef _WIN32
  char *tmpPath = getenv("TMP");
  string str(tmpPath);
  if (str.substr(str.size() - 1, 1) != "\\")
    str += "\\";
  return str;
#else
  char *tmpPath = getenv("TMP");
  if (!tmpPath || !*tmpPath)
    return "/tmp/";
  string str(tmpPath);
  if (str.substr(str.size() - 1, 1) != "/")
    str += "/";
  return str;
#endif
}


void createtempfile(ofstream  &fileStream, string &filePath, std::ios_base::openmode flags)
{
#ifdef _WIN32
  char buffer[BUFSIZ];
  ::GetTempFileNameA(gettempfolder().c_str(), "", 0, buffer);
  filePath = buffer;
#else
  string tmpfolder = gettempfolder();
  char buffer[tmpfolder.size() + 16];
  strcpy(buffer, tmpfolder.c_str());
  strcat(buffer, "dskbuff--XXXXXX");
  mkstemp(buffer);
  filePath = buffer;
#endif
  fileStream.open(filePath.c_str(), flags);
}

void removefile(const std::string &filePath)
{
#ifdef _WIN32
  ::DeleteFileA(filePath.c_str());
#else
  if (remove(filePath.c_str()) != 0)
    perror("Error deleting file" );
#endif
}



inputfilestream::inputfilestream(const std::string &filePath)
  : std::istream(0),
    m_streambuf(0)
{
  //check if file is readable
  std::filebuf* fb = new std::filebuf();
  _good=(fb->open(filePath.c_str(), std::ios::in)!=NULL);

  if (filePath.size() > 3 &&
      filePath.substr(filePath.size() - 3, 3) == ".gz") {
    fb->close();
    delete fb;
    m_streambuf = new gzfilebuf(filePath.c_str());
  } else {
    m_streambuf = fb;
  }
  this->init(m_streambuf);
}

inputfilestream::~inputfilestream()
{
  delete m_streambuf;
  m_streambuf = 0;
}

void inputfilestream::close()
{
}



/* MemoryMap Management
Code kindly provided by Fabio Brugnara, ITC-irst Trento.
How to use it:
- call MMap with offset and required size (psgz):
  pg->b = MMap(fd, rdwr,offset,pgsz,&g);
- correct returned pointer with the alignment gap and save the gap:
  pg->b += pg->gap = g;
- when releasing mapped memory, subtract the gap from the pointer and add
  the gap to the requested dimension
  Munmap(pg->b-pg->gap, pgsz+pg->gap, 0);
*/


void *MMap(int	fd, int	access, off_t	offset, size_t	len, off_t	*gap)
{
  void	*p;
  int	pgsz,g=0;

#ifdef _WIN32
  /*
  // code for windows must be checked
  	HANDLE	fh,
  		mh;

  	fh = (HANDLE)_get_osfhandle(fd);
      if(offset) {
        // bisogna accertarsi che l'offset abbia la granularita`
        //corretta, MAI PROVATA!
        SYSTEM_INFO	si;

        GetSystemInfo(&si);
        g = *gap = offset % si.dwPageSize;
      } else if(gap) {
        *gap=0;
      }
  	if(!(mh=CreateFileMapping(fh, NULL, PAGE_READWRITE, 0, len+g, NULL))) {
  		return 0;
  	}
  	p = (char*)MapViewOfFile(mh, FILE_MAP_ALL_ACCESS, 0,
                             offset-*gap, len+*gap);
  	CloseHandle(mh);
  */

#else
  if(offset) {
    pgsz = sysconf(_SC_PAGESIZE);
    g = *gap = offset%pgsz;
  } else if(gap) {
    *gap=0;
  }
  p = mmap((void*)0, len+g, access,
           MAP_SHARED|MAP_FILE,
           fd, offset-g);
  if((long)p==-1L) {
    perror("mmap failed");
    p=0;
  }
#endif
  return p;
}


int Munmap(void	*p,size_t	len,int	sync)
{
  int	r=0;

#ifdef _WIN32
  /*
    //code for windows must be checked
  	if(sync) FlushViewOfFile(p, len);
  	UnmapViewOfFile(p);
  */
#else
  cerr << "len  = " << len << endl;
  cerr << "sync = " << sync << endl;
  cerr << "running msync..." << endl;
  if(sync) msync(p, len, MS_SYNC);
  cerr << "done. Running munmap..." << endl;
  if((r=munmap((void*)p, len))) perror("munmap() failed");
  cerr << "done" << endl;

#endif
  return r;
}


//global variable
Timer g_timer;


void ResetUserTime()
{
  g_timer.start();
};

void PrintUserTime(const std::string &message)
{
  g_timer.check(message.c_str());
}

double GetUserTime()
{
  return g_timer.get_elapsed_time();
}




int parseWords(char *sentence, const char **words, int max)
{
  char *word;
  int i = 0;

  const char *const wordSeparators = " \t\r\n";

  for (word = strtok(sentence, wordSeparators);
       i < max && word != 0;
       i++, word = strtok(0, wordSeparators)) {
    words[i] = word;
  }

  if (i < max) {
    words[i] = 0;
  }

  return i;
}


//Load a LM as a text file. LM could have been generated either with the
//IRST LM toolkit or with the SRILM Toolkit. In the latter we are not
//sure that n-grams are lexically ordered (according to the 1-grams).
//However, we make the following assumption:
//"all successors of any prefix are sorted and written in contiguous lines!"
//This method also loads files processed with the quantization
//tool: qlm

int parseline(istream& inp, int Order,ngram& ng,float& prob,float& bow)
{

  const char* words[1+ LMTMAXLEV + 1 + 1];
  int howmany;
  char line[MAX_LINE];

  inp.getline(line,MAX_LINE);
  if (strlen(line)==MAX_LINE-1) {
    cerr << "parseline: input line exceed MAXLINE ("
         << MAX_LINE << ") chars " << line << "\n";
    exit(1);
  }

  howmany = parseWords(line, words, Order + 3);

  if (!(howmany == (Order+ 1) || howmany == (Order + 2)))
    assert(howmany == (Order+ 1) || howmany == (Order + 2));

  //read words
  ng.size=0;
  for (int i=1; i<=Order; i++)
    ng.pushw(strcmp(words[i],"<unk>")?words[i]:ng.dict->OOV());

  //read logprob/code and logbow/code
  assert(sscanf(words[0],"%f",&prob));
  if (howmany==(Order+2))
    assert(sscanf(words[Order+1],"%f",&bow));
  else
    bow=0.0; //this is log10prob=0 for implicit backoff

  return 1;
}
