/*
 *  TRich_Analysis.h
 *  
 * Read and analysis of single run or scan (many consecutive runs)
 * include scalers thr scan and single TDC data analysis
 * 
 */

#ifndef TRICH_ANALYSIS_H
#define TRICH_ANALYSIS_H

#include <TH1F.h>
#include "TRich_Scaler.h"

#define TREE_SCALER_THR "Tdata" //  name of the tree
#define TREE_TDC "Tdata" //  name of the tree

#define NMAXTHR 1024 // 10 bit DAC0 register 

#define SINGLE_RUN 0
#define SCAN 1


using namespace std;

class TRich_Analysis {

private:

  // august '15 revision
  int fdaq_mode;		// 0 scaler; 1 tdc
	int fmode;			// single run or scan

  int fTDC_repetition;
  int fTDC_runID_first;
  int fTDC_runID_last;


  int frepetition;
  int frunID_first;
  int frunID_last;

  string frun_path;
  string frun_file;
  string frun_name;
	string frun_prefix; 
  int		 frun_id;	
	string frun_suffix; 

	string fTDCroot;
	string fTDCscanroot;

	string fTDCpdf;
  string fTDCscanpdf;

  string fthrscan_eps_all; 
  
  string fsource ; // Laser, Dark, Background
  string fplot; // Rate, Efficiency
  int fnorm;// Duration [s], or Laser Rate [Hz] 
  
  // first version
  string finraw;
  string finlog;
  string fintxt;
  string fioTDCroot; 
  string fioSKAroot_Scan;
  string fprefix;
 // string fPlotTimeStamp;
 // string fPlotTimeStampIntervals;
  
 // string fPlotChannel; // temporary, the name must change 
 // string fPlotEdge; // ...same
 // string fPlotTime; //..same
  //string fPlotTime_SingleChannel;
  
 
  int fposedge[192];	// equivalent of scaler 
  double feff[192];		// counts/nevents


  
 public:
  
  TRich_Analysis(const char * fileraw = NULL);
  ~TRich_Analysis();

	void Ingest();
  
	bool SingleRun();

	bool Scan();




	std::string inPath(){return frun_path;}
	std::string inPrefix(){return frun_prefix;}

  void SetDaqMode(int daqmode); // 0 for scaler analysis, 1 for tdc analysis
  void SetSource(string sourceName=" Laser, Dark or Background ?",string plotName="Rate or Efficiency?",int normalization=1);
  
  void Print();
  int SetNames(string runPrefix = NULL,int runID_first =0,int runID_last=0,string runPath = "../data/out/");
  int NameRun(const char * fileraw = NULL,int runID_last=0,bool enablePrint = false);
  
  unsigned int	GetThreshold();
  unsigned int	GetTriggerDelay();
  unsigned int	GetEventBuilderLookBack();	
  unsigned int	GetEventBuilderWindow();
  unsigned int	GetGain();

	
  
  double GetEfficiency(int channel = 0);	
  
  // TDC
  int		TDC_Read();
	void 	ProcessTDC();
	void 	ProcessTDCTEMP(); // BACK UP copy during sw update
	void 	ProcessTDCscan();
  void	TDC_Plot(int asic = 0,bool draw_single_channel= false);	
  void	TDC_Process_SingleEvent();
  
  
  // scaler
  TRich_Scaler *  SKA_Read();
  int SKA_Read_Threshold_Scan();
  int SKA_Plot2(float minY=.1,float maxY=1.1);
  int SKA_Plot3(float minZ=.1, float maxZ=1.1,int DAC0step=1);	

 private:
  double DecodeTimeStamp(unsigned int tstamp0=0,unsigned int tstamp1=0);	
  unsigned int GetFPGAParameters(int opt = -1);
  void TDC_Draw(TH1F * hRise ,TH1F * hFall);



};
#endif