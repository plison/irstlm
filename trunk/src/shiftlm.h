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

namespace irstlm {

// Non linear Shift based interpolated LMs

class shiftone: public mdiadaptlm
{
protected:
  int prunethresh;
  double beta;
public:
  shiftone(char* ngtfile,int depth=0,int prunefreq=0,TABLETYPE tt=SHIFTBETA_B);
  int train();
  int discount(ngram ng,int size,double& fstar,double& lambda,int cv=0);
  ~shiftone() {}
};


class shiftbeta: public mdiadaptlm
{
protected:
  int prunethresh;
  double* beta;

public:
  shiftbeta(char* ngtfile,int depth=0,int prunefreq=0,double beta=-1,TABLETYPE tt=SHIFTBETA_B);
  int train();
  int discount(ngram ng,int size,double& fstar,double& lambda,int cv=0);
  ~shiftbeta() {
    delete [] beta;
  }

};


class symshiftbeta: public shiftbeta
{
public:
  symshiftbeta(char* ngtfile,int depth=0,int prunefreq=0,double beta=-1):
    shiftbeta(ngtfile,depth,prunefreq,beta) {}
  int discount(ngram ng,int size,double& fstar,double& lambda,int cv=0);
};

	
	class mshiftbeta: public mdiadaptlm
	{
	protected:
		int prunethresh;
		double beta[3][MAX_NGRAM];
		ngramtable* tb[MAX_NGRAM];
		
		double oovsum;
		
	public:
		mshiftbeta(char* ngtfile,int depth=0,int prunefreq=0,TABLETYPE tt=MSHIFTBETA_B);
		int train();
		int discount(ngram ng,int size,double& fstar,double& lambda,int cv=0);
		
		~mshiftbeta() {}
		
		int mfreq(ngram& ng,int l) {
			return (l<lmsize()?getfreq(ng.link,ng.pinfo,1):ng.freq);
		}
		
		double unigrMSB(ngram ng);
		inline double unigr(ngram ng){ return unigrMSB(ng); };		
	};
	
class quasi_mshiftbeta: public mdiadaptlm
{
protected:
  int prunethresh;
  double beta[3][MAX_NGRAM];
  ngramtable* tb[MAX_NGRAM];

  double oovsum;

public:
  quasi_mshiftbeta(char* ngtfile,int depth=0,int prunefreq=0,TABLETYPE tt=QUASI_MSHIFTBETA_B);
  int train();
  int discount(ngram ng,int size,double& fstar,double& lambda,int cv=0);

  ~quasi_mshiftbeta() {}

  inline int mfreq(ngram& ng,int /*NOT_USED l*/) { return ng.freq; }

};
	
}//namespace irstlm
