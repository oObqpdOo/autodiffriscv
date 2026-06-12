#include "Tinn_fw.h"
#include "cdata_ridiculous_short.h"
#include "cres_ridiculous_short.h"

// Randomly shuffles a data object.
static void shuffle(void){
    for(int a = 0; a < ROWS; a++)
    {
#ifdef USE_RAND        
	const int b = rand() % ROWS;
#else
	const int b = get_random() % ROWS;
#endif
        short ot = res_arr[a];
        char it[COLS];
        for(int i = 0; i < COLS; i++){
	        it[i] = data_arr[a][i];
		}
        // Swap output.
        res_arr[a] = res_arr[b];
        res_arr[b] = ot;
        // Swap input.
        for(int i = 0; i < COLS; i++){
	        data_arr[a][i] = data_arr[b][i];
 	    	data_arr[b][i] = it[i];
    	}
    }
}

// Learns and predicts hand written digits with 98% accuracy.
int main()
{
    // Tinn does not seed the random number generator.
#ifdef USE_RAND
#ifndef USE_CONST_SEED
    srand(time(0));
#else
    srand(23);
#endif
#else 
    init_lfsr(0xABDEC);
#endif
    // Hyper Parameters.
    // Learning rate is annealed and thus not constant.
    // It can be fine tuned along with the number of hidden layers.
    // Feel free to modify the anneal rate.
    // The number of iterations can be changed for stronger training.
    float rate = 1.0f;
    int nhid = 28;
    float anneal = 0.99f;
    int iterations = 10;//128;
    // Train, baby, train.
    Tinn tinn = xtbuild(NIPS, nhid, NOPS);
    //local float copies for one element each (256 input parameters 10 output parameters)
    static float data_in[NIPS];
    static float data_tg[NOPS];
    for(int i = 0; i < iterations; i++)
    {
        shuffle();
        float error = 0.0f;
        for(int j = 0; j < ROWS; j++)
        {
        	for (int k = 0; k < NIPS; k++){
        		//int localindex = (int)(k/sizeof(char));
        		int shift = (k%8);
        		float local_value = (float)(CHECK_BIT((data_arr[j][(int)(k/8)]), (7-shift)));
        		//printf("%d %d (%x) shift: %d: %f\n", j, (int)(k/8), data_arr[j][(int)(k/8)], shift, local_value);
        		data_in[k] = local_value;
    		}
        	for (int l = 0; l < NOPS; l++){
        		float local_value = (float)(CHECK_BIT(res_arr[j],(NOPS-1-l))); 
        		//printf("%d (%x): %f\n", j, res_arr[j], local_value);
        		data_tg[l] = local_value;	
            }
            error += xttrain(tinn, data_in, data_tg, rate);
        }
        /*printf("error %.12f :: learning rate %f\n",
            (double) error / ROWS,
            (double) rate);*/
        float error_p_r = (float)(error/ROWS);
        //xtprint(&error_p_r, 1, 0);
        //xtprint(&rate, 1, 1);
        rate *= anneal;
    }
    // This is how you save the neural network to disk.
    //xtsave(tinn, "saved.tinn");
    //xtfree(tinn);
    // This is how you load the neural network from disk.
    //Tinn loaded = xtload("saved.tinn");
    // Now we do a prediction with the neural network we loaded from disk.
    // Ideally, we would also load a testing set to make the prediction with,
    // but for the sake of brevity here we just reuse the training set from earlier.
    // One data set is picked at random (zero index of input and target arrays is enough
    // as they were both shuffled earlier).
   	
	for (int k = 0; k < NIPS; k++)
		data_in[k] = (float)(CHECK_BIT((data_arr[0][(int)(k/8)]), (7-(k%8))));
	for (int l = 0; l < NOPS; l++)
		data_tg[l] = (float)(CHECK_BIT(res_arr[0],(NOPS-1-l)));	
   	
    float* pd = xtpredict(tinn, data_in);
    
    xtprint(&error_p_r, 1, 0);
    xtprint(&rate, 1, 1);
    // Prints target.
    xtprint(data_tg, NOPS, 2);
    // Prints prediction.
    xtprint(pd, NOPS, 12);
    // All done. Let's clean up.
    
    xtfree(tinn);
    return 0;
}
