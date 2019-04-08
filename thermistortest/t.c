#include <stdio.h>
#include <stdlib.h>

//Calculated with this table https://www.makeralot.com/download/Reprap-Hotend-Thermistor-NTC-3950-100K.pdf
//and with the follwing equations 
//Voltage on ADC = 5 * ( 10K / (Thermistor + 10K) )
//ADC value = (Voltage on ADC) / (5 / 1024)

//Very accurate (1 degree) around printing temperatures, within usually 5 degree C in lower temperatures (30 - 140)

//table [ADC Value][Temperature]
const int table[34][2] = {
	{30,0},
	{76,20},
	{93,25},
	{113,30},
	{223,50},
	{414,75},
	{613,100},
	{827,140},
	{853,145},
	{870,150},
	{884,155},
	{897,160},
	{909,165},
	{921,170},
	{931,175},
	{940,180},
	{948,185},
	{955,190},
	{962,195},
	{968,200},
	{972,205},
	{977,210},
	{981,215},
	{985,220},
	{988,225},
	{991,230},
	{994,235},
	{997,240},
	{1001,250},
	{1003,255},
	{1004,260},
	{1006,265},
	{1007,270},
	{1010,280}
};


int main(){
	int inputV, i, T, R, bigA, smallA, bigT, smallT;
	float Tslope, calV;
	char done = 0;
	
	while(1){
		printf("Please enter an ADC value: ");
		scanf("%d", &inputV);
		
		//Check for exact match
		for (i = 0; (i < 34) && (done == 0); i++){
				if (inputV == table[i][0]){
					printf("\nExact match -> Temperature = %d C\n\n", table[i][1]);
					done = 1;
				}
		}
		
		//Check if too cold
		if (inputV < 30){
			done = 1;
			T = 0;
			printf("\nToo cold!\n\n");
		}
			
		//Check if too hot
		if (inputV > 1010){
			done = 1;
			T = -1;
			printf("\nToo hot!\n\n");
		}
		
		//If no exact match, do interpolation
		if (done == 0){
			
			//Find ranges
			for (i = 0; (i < 34) && (done == 0); i++){
					if (table[i][0] > inputV){
						bigA = table[i][0];
						bigT = table[i][1];
						smallA = table[i - 1][0];
						smallT = table[i - 1][1];
						done = 1;
					}
			}
			
			//Calculate T
			Tslope = (float)((float)(bigT-smallT)/(float)(bigA-smallA));
			//printf("\nY2 = %d, Y1 = %d, X2 = %d, X1 = %d, Slope = %f\n", bigT, smallT, bigA, smallA, Tslope);
			T = Tslope * inputV + (smallT - Tslope * smallA);
			
			//Calculate R
			calV = inputV * (0.0048828125);
			R = (float)(50000 - (10000 * calV)) / calV;
			
			printf("\nInterpolation -> Temperature = %d C", T);
			printf("\n                 Resistance  = %d\n\n", R);
		}
		
		done = 0;
		
	}
	
	return 0;
}