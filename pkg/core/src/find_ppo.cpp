#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
//#include <omp.h>
#include "cell_mgr.h"
#include "circuit_builder.h"
#include "circuit.h"
#include "pattern_set.h"
#include "circuit_simulator.h"
#include "library.h"
#include "library_parser.h"
using namespace std;

int main(int argc , char ** argv)
{
	
	// set timer
	clock_t t1, t2;
	t1 = clock();

	// check files
	if(argc < 4)
	{
		cout << "please input verlog, sdf and pat file" <<endl;
		return 0;
	}

	// ================================================================================
	// cell manager contain nangate45 library information(pin name and cell type names)
	// ================================================================================
	Library lib;
	LibraryParser libp(&lib);
	if(!libp.read(argv[1]))
	{
		cout << "**ERROR main(): LIB parser failed" <<endl;
		return 0;
	}
	
	// ===============================================================================
	// use circuit builder to build a circuit
	// ===============================================================================
	
	CircuitBuilder cb;
	
	// tell circuit builder to build the circuit using cells in cell manager
	cb.set(&lib);
	
	// tell circuit builder to build the levelized circuit whose clk is set to level 0
	cb.set("CK");
	
	// set flip flop functional pins
	cb.set("Q","QN","D","SI","SE","CK");
	
	// read the circuit
	if(!cb.read(argv[2]))
	{
		cout << "read verlog file fail" <<endl;
		return 0;
	}

	// get the circuit
	Circuit cir = cb.getCircuit();
	
	// ===============================================================================
	// use pattern set to read the pattern
	// ===============================================================================
	cout << "read pattern" <<endl;
	PatternSet ps;
	
	// tell pattern set to build ppi and pi order according to cell ID in the circuit
	ps.set(&cir);
	
	// read the pattern file
	if(!ps.setFile(argv[4]))
	{
		cout << "read pattern file fail" <<endl;
		return 0;
	}
	ps.readAll();

	// ===============================================================================
	// use circuit simulator to simulate the circuit
	// ===============================================================================
	
	CircuitSimulator cs;
	
	// tell which circuit will be simulated
	cs.set(&cir);
	
	// set cell manager to get pin name when simulation
	cs.set(&lib);

	// set clk wave of the simulation
	Wave clkWave;
	clkWave.initialValue = Wave::L;
	
	// set rise transition of clk
	Transition riseTrans;
	riseTrans.value = Wave::H;
	riseTrans.time = 0.0;
	riseTrans.period = 0.002;
	riseTrans.prevTransition = NULL;
	clkWave.addTransition(Wave::H , riseTrans);//0.0 , 0.002, NULL, 0);
	
	// set fall transition of clk
	Transition fallTrans;
	fallTrans = riseTrans;
	fallTrans.value = Wave::L;
	fallTrans.time = 0.25;
	clkWave.addTransition(Wave::L , fallTrans);//0.25 , 0.002, NULL, 0);
	
	cs.set("CK", &clkWave , 1.0);


	// set pattern pi and ppi order
	cs.set(&ps.piOrder , &ps.poOrder , &ps.ppiOrder);
	// ===============================================================================
	// perform circuit simulation
	// ===============================================================================
	cout << "start!" << endl;
	for(unsigned i = 0 ; i < ps.patterns.size() ; ++i)
	{
		// system("clear");
		Pattern pat = ps.patterns[i];
		
		cs.initial(pat.pis[0] , pat.ppi);
		string ppo = cs.getPPO();
		if(pat.cycle[0] == Pattern::CAPTURE)
			cout << "pattern " << i << "\t" <<ppo << endl;
		else
			cout << "pattern " << i << "\t" << pat.ppi << endl;

	}
	//}
	t2 = clock();
	cout << "total cost: " <<  (t2-t1)/(double)(CLOCKS_PER_SEC)/12 << " second!" <<endl;

	ofstream fout;
	fout.open("time.txt",ios::app);
	fout << cir.getName() << ","  << cir.getNets().size() << "," << ps.patterns.size()<< ","<< (t2-t1)/(double)(CLOCKS_PER_SEC)/12 << endl;
	return 0;
}
