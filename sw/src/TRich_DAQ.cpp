/*
 *  TRich_DAQ.cpp
 *  
 *
 *  Created by Matteo Turisini on 28/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h> // printf
#include <iostream> // cerr, endl
#include <signal.h>	// SIGINT, signal
#include <time.h>       /* time_t, struct tm, time, localtime */
#include <stdint.h>  

#include "TRich_DAQ.h"

/* Signal Handling*/
static int sig_int=1;
void ctrl_c(int){
	sig_int =0;
	printf("\n CTRL-C pressed\n\r");
}


/* TRich_DAQ class*/
TRich_DAQ::TRich_DAQ(){
		
	fcfg = NULL;
	ffe = NULL ;
nmeas =0;
	fprogress_old=0;

}


TRich_DAQ::~TRich_DAQ(){
}


 
void TRich_DAQ::SetFrontend(TRich_Frontend * fe){
  if ( fe==NULL) {cout << "Warning: NO front end"<<endl;}
  ffe = fe ;   
}
void TRich_DAQ::SetConfiguration(TRich_Config * cfg){
  if (cfg==NULL) {cout << "Warning: NO configuration"<<endl;}	
  fcfg = cfg;	
}


/*time in nanosecond in POSIX systems*/

void 	TRich_DAQ::TimeStart(){
	
#if __APPLE__	
  printf("I am on Mac OsX\n");
  //*pt = mach_absolute_time();// pt is a uint64_t
#else
  //	printf("I am on Linux\n");

	uint64_t t;
	uint64_t t_ns;

	clock_gettime(CLOCK_REALTIME,&fstart);
//	printf("Time Start is %ld sec + %ld nsecond\n",fstart.tv_sec,fstart.tv_nsec);

 	t = fstart.tv_sec;
 	t_ns = fstart.tv_nsec;
//	printf("Time Start: %12ld sec + %12ld nsecond\n",t,t_ns);


#endif

}
float 	TRich_DAQ::TimeNow(){


float lduration = 0.0;
nmeas++;

#if __APPLE__
	printf("I am on Mac OsX\n");
#else
	//	printf("I am on Linux\n");

	uint64_t t;
	uint64_t t_ns;
	timespec mark1;
	
	clock_gettime(CLOCK_REALTIME,&mark1);

 	t = mark1.tv_sec;
 	t_ns = mark1.tv_nsec;
  //printf("Time Now  : %12ld sec + %12ld nsecond ",t,t_ns);


	uint64_t duration;
	uint64_t duration_ns; 

	if(t_ns>(uint64_t)fstart.tv_nsec){
		duration = t - fstart.tv_sec;
		duration_ns =t_ns -fstart.tv_nsec ; 

	}else{
		duration = t - fstart.tv_sec-1;
		duration_ns = GIGA + t_ns -fstart.tv_nsec; 
	}

//	printf("Duration  : %12ld sec + %12ld nsecond\n",duration,duration_ns);
//	printf("Time Now  : %12ld sec + %12ld nsecond\n",duration,duration_ns);

	lduration = (float)duration + ((float) duration_ns)/GIGA; 

//printf("%d %lf\n",nmeas,fduration);
 
#endif
	

return lduration;
	
}


/*date and hour */

string TRich_DAQ::TimeString(){

	time_t	ltref;	
	time(&ltref);
	struct tm * ltim = localtime(&ltref);
	//printf ("Current time and date: %s", asctime(ltim));

	string dateStr =  asctime(ltim);
	//printf ("Current time and date: %s", dateStr.c_str());

	return dateStr;
}

void TRich_DAQ::PreEv(){

	// open outfile
	fraw = this->OpenOutFile(fcfg->RunPrefix());
	
	// print date on screen
	printf ("Date: %s", this->TimeString().c_str());// time and date

	this->TimeStart();
}


long long  TRich_DAQ::DoEv(int verbosity){
		
  signal(SIGINT,ctrl_c); // if receives ctrl-c from the keyboard execute ctrl_c
	
  //unsigned int gBuf[1000000]; //obsolete
  
  int val = 0;
  //int nevts = 0;
  //int poll = 0;
 // int evt=0;
 // int operationID =0;
  
	// read DAQ settings
  int mode	   	= fcfg->Daq_Mode();
  int event_preset 	= fcfg->EventPreset() ;
  int time_preset  	= fcfg->TimePreset(); 
  int iteration    	= fcfg->ScalerRepetition();
  int duration    	= fcfg->ScalerDuration();
  
  // new TDC socket
  int n;
  long long nevents=0; 
  long long  nbytes = 0;
  long long dbytes = 0;;

 // int dummy=0;

  ffe->StartDAQ();
  
  switch(mode){

  case MODE_TDC:
  //  nevents = 0;
  //  nbytes = 0;
  //  dbytes = 0;
    printf("RICH - waiting for TDC event data...current run will end in %d seconds or press ctrl-c\n",time_preset );

    do{
      n = ffe->Receive(&val);
     
			if((val==0)||(n==-1)){
		//		printf("Received nothing\n");
				continue;
			}else{
	//			printf("Received %d bytes, val = 0x%X\n",n,val);
			}

			//if(((val & 0xF8000000) == 0x90000000)&&(nevents>=event_preset)){
				//	printf("HEADER found\n");			
				//break;
      //}
     
		 fwrite(&val, sizeof(val), 1, fraw);
     
      nbytes+= sizeof(val);
      dbytes+= sizeof(val);
      
      if((val & 0xF8000000) == 0x90000000){
				nevents++;
				//ProgressBar();			
				
			if(!(nevents % ((long long)(event_preset/10.)))){
	    
				printf("[0x%08X] nevents = %lld, nbytes = %lld, dbytes = %lld\n", val, nevents, nbytes, dbytes);
	    		fflush(stdout);
	   			dbytes = 0;     
	 		 }

      }
      fduration = this->TimeNow(); 
    }while(fduration<time_preset&&sig_int&&nevents<=event_preset);	  
    
    break;
    
  case MODE_SCALER:
    
		printf("SCALER: Duration %d seconds, repetition %d \n",duration,iteration);
		ffe->DumpScalers(); // first measure skipped

		for(int i=0;i<iteration;i++) {
			
			sleep(duration);
			nevents += ffe->DumpScalers(fraw,true);

			// estrarre valori medi scalers asic 0, 1, 2!!
			if(sig_int==0){break;} 
		}
		break;
	    
  default: 
    printf("Unknown DAQ mode, please use 0 for scalers, 1 for TDC\n");
    break;
  }
  printf("\n");	
  
	fduration = this->TimeNow();

	fnevts= nevents;



	return nevents;
}

void TRich_DAQ::PostEv(){

	fclose(fraw);

	printf ("Date         : %s", this->TimeString().c_str());// time and date
  	printf("Events[#]     : %6d \n",fnevts);
	printf("Duration [s]  : %6.3lf\n",fduration);
	printf("Rate [evts/s] : %6.0lf\n",fnevts/fduration);
	printf("DAQ Completed and data successfully written to %s \n",frunName.c_str());


	// daq statistics are exported
  fcfg->Logbook("Events",fnevts);
  fcfg->Logbook("Duration",fduration);
	fcfg->Export(flogName.c_str());
}


void  	TRich_DAQ::ProgressBar(){

  int event_preset 	= fcfg->EventPreset() ;
  int time_preset  	= fcfg->TimePreset(); 

	//printf("Durata attuale %lf ",fduration);
	//printf("preset tempo %d ",time_preset);
	//printf("ratio  %lf\n",fduration/time_preset);
	//printf("ratio x step  %lf\n",(fduration/time_preset)*PROGRESS_STEP);

	int progress_time = (fduration/time_preset)*PROGRESS_STEP; 
	
	//printf("Eventi Attuali %d ",fnevts);
	//printf("preset eventi  %d ",event_preset);
	//printf("ratio evt %lf\n",((float)fnevts)/event_preset);
	//printf("ratio evt x step  %lf\n",(fnevts/event_preset)*PROGRESS_STEP);

	int progress_evt = (((float) fnevts)/event_preset)*PROGRESS_STEP;
	
	int progress = progress_time > progress_evt ? progress_time : progress_evt;

	progress = progress < PROGRESS_STEP ? progress : PROGRESS_STEP;

	//printf("progress time %d\n",progress_time);
	//printf("progress evt  %d\n",progress_evt);
	//printf("progress      %d\n",progress);
	
	for (int jprog = 0; jprog<(progress - fprogress_old); jprog++)
	{
		if (progress_time > progress_evt) 
		{
			printf("T");
		} else {
			printf("E");
		}
		fflush(stdout);
	}
	
	if (progress == PROGRESS_STEP){} // 
	fprogress_old = progress;

}


string TRich_DAQ::GetRunName(){return frunName;}
string TRich_DAQ::GetLogName(){return flogName;}


int TRich_DAQ::ReadRunNum(){

	FILE *ff;
	int rr;
	ff = fopen(MRC_RUN_NUMBER_FILE,"r");
	if (ff) {
		fscanf(ff,"%d",&rr);
		fclose(ff);
	} else {
		rr = -1;
	}
	return rr;
	
}
int TRich_DAQ::WriteRunNum(int rr){
	FILE *ff;
	ff = fopen(MRC_RUN_NUMBER_FILE,"w");
	if (ff) {
		fprintf(ff,"%d",rr);
		fclose(ff);
	} else {
		printf("WARNING: cannot update run file %s",MRC_RUN_NUMBER_FILE);
	}
	return 0;
}
FILE * TRich_DAQ::OpenOutFile(std::string file_prefix){

	
	FILE *fp;	
	int irun = this->ReadRunNum()+1;
	this->WriteRunNum(irun);
	
	char numstr[6]; // enough to hold all numbers up to 6 digits
	sprintf(numstr, "_%06d", irun);
	
	// binary output file 
	frunName.clear();
	frunName += OUT_PATH;
	frunName += file_prefix;	
	frunName += numstr;
	frunName += SUFFIX_RAW;
	
	// logbook output file 
	flogName.clear();
	flogName += OUT_PATH;
	flogName += file_prefix;	
	flogName += numstr;
	flogName += SUFFIX_LOG;

	// plain text output file
	
	ftxtName.clear();
	ftxtName += OUT_PATH;
	ftxtName += file_prefix;	
	ftxtName += numstr;
	ftxtName += SUFFIX_TXT;
	
	
	if ( (fp=fopen(frunName.c_str(), "wb")) == NULL ){
		printf("Cannot open output file %s\n",frunName.c_str());
		return NULL;
	}
/*
	printf("Binary file %s\n",frunName.c_str());
	printf("Logbookfile %s\n",flogName.c_str());
	printf("Text   file %s\n",ftxtName.c_str());
	*/
	return fp;
	
}
