import gzip
import pandas as pd # type: ignore
import numpy as np # type: ignore


###
### GENERAL PURPOSE UTILITY FUNCTIONS
### 
def generate_all_bitstrings(n):
    if (n==1):
        return ['0','1']
    else:
        suffixes = generate_all_bitstrings(n-1)
        return ['0'+s for s in suffixes]+['1'+s for s in suffixes]

###
### XCSLIB UTILITY FUNCTIONS
###
def load_xcslib_population(filename):
    with gzip.open(filename, 'rb') as INPUT:
        population_text = INPUT.read()
        
    classifiers_text = population_text.decode("utf-8").split("\n")    
    classifiers = [c.strip().split("\t") for c in classifiers_text if c.strip()!=""]
        
    if len(classifiers[0])==9:
        # simple population
        df = pd.DataFrame(columns = ['id','condition','action', 'prediction','error','fitness','action_set_size','experience','numerosity'], data = classifiers)    

        df['prediction'] = df['prediction'].astype(float)
        df['error'] = df['error'].astype(float)
        df['fitness'] = df['fitness'].astype(float)
        df['action_set_size'] = df['action_set_size'].astype(float)
        df['experience'] = df['experience'].astype(int)
        df['numerosity'] = df['numerosity'].astype(int)
        
    elif len(classifiers[0])==13:
        # population with niche information
        df = pd.DataFrame(columns = ['id','condition','action', 'prediction','error','fitness','action_set_size','experience','numerosity', 
                                    'creation time stamp', 'time stamp', 'as time stamp', 'niches time stamps'], data = classifiers)    

        df['prediction'] = df['prediction'].astype(float)
        df['error'] = df['error'].astype(float)
        df['fitness'] = df['fitness'].astype(float)
        df['action_set_size'] = df['action_set_size'].astype(float)
        df['experience'] = df['experience'].astype(int)
        df['numerosity'] = df['numerosity'].astype(int)
        df['creation time stamp'] = df['creation time stamp'].astype(int)
        df['time stamp'] = df['time stamp'].astype(int)
        df['as time stamp'] = df['as time stamp'].astype(int)

    else:
        raise Exception('Population with '+len(classifiers[0])+' columns not supported')
    return df 

def load_xcslib_statistics(statistics_filepath):
    column_names_ms = {0:'Experiment',\
        1:'Trial',\
        2:'# Steps',\
        3:'Reward',\
        4:'Population Size',\
        5:'Problem Type'}

    column_names_ss = {0:'Experiment',\
        1:'Trial',\
        2:'# Steps',\
        3:'Reward',\
        4:'Population Size',\
        5:'System Error',\
        6:'Problem Type'}

    if (statistics_filepath[-3:]==".gz"):
        df = pd.read_csv(statistics_filepath, header=None, sep="\t", compression="gzip")
    else:
        df = pd.read_csv(statistics_filepath, header=None, sep="\t")

    if (len(df.columns)==6):
        df.rename(columns=column_names_ms,inplace = True)
    elif (len(df.columns)==7):
        df.rename(columns=column_names_ss,inplace = True)
    else:
        raise Exception("ERROR: wrong number of columns ("+str(len(df.columns))+") it should be 6 or 7")
    return df

def match(condition,state,dontcare_symbol='#'):
    """returns true if the state matches the condition."""
    if (len(condition)!=len(state)):
        raise Exception("SIZE MISMATCH CONDITION SIZE=%d STATE SIZE=%d"%(len(condition),len(state)))
        # return False
            
    for i in range(len(condition)):
        if (condition[i]!=dontcare_symbol) and (condition[i]!=state[i]):
            return False
    return True  



###
### OPTIMAL GENERALIZATION FUNCTIONS
###
def compute_cluster_prediction_statistics(df_cluster):
    """Compute the prediction statistics for each cluster."""
    # consider only the actual clusters not the noise points
    labels = df_cluster['label'].unique()
    real_labels = np.delete(labels,np.where(labels == -1))
    df_real_clusters = df_cluster[df_cluster['label'] != -1].copy()
    
    prediction_statistics = {}
    df_prediction_statitics = {
            'cluster':[],
            'average':[],
            'std':[],
            'median':[],
            'first quantile':[],
            'third quantile':[],
            'max':[],
            'min':[],            
    }
    
    for label in real_labels:
        df_single_cluster = df_real_clusters[df_real_clusters['label']==label].copy()

        prediction_vector = generate_prediction_vector(df_single_cluster)

        mean_prediction = np.mean(prediction_vector)
        std_prediction = np.std(prediction_vector)

        median_prediction = np.median(prediction_vector)
        fq_prediction = np.percentile(prediction_vector,0.25)
        tq_prediction = np.percentile(prediction_vector,0.75)

        prediction_statistics[label]={
            'cluster':label,
            'average':mean_prediction,
            'std':std_prediction,
            'median':median_prediction,
            'first quantile':fq_prediction,
            'third quantile':tq_prediction,
            'max':np.max(prediction_vector),
            'min':np.min(prediction_vector),        
        }
        
        df_prediction_statitics['cluster'].append(label)
        df_prediction_statitics['average'].append(mean_prediction)
        df_prediction_statitics['std'].append(std_prediction)
        df_prediction_statitics['median'].append(median_prediction)
        df_prediction_statitics['first quantile'].append(fq_prediction)
        df_prediction_statitics['third quantile'].append(tq_prediction)
        df_prediction_statitics['max'].append(np.max(prediction_vector))
        df_prediction_statitics['min'].append(np.min(prediction_vector))
        
    return prediction_statistics, pd.DataFrame.from_dict(df_prediction_statitics)

def generate_prediction_vector(population, use_numerosity=True):
    """Generate the prediction vector from the population by 
    repeating each prediction values for numerosity."""

    if (not use_numerosity):
        return population.values.reshape(-1,1)
    
    predictions = []
    
    for i,row in population.iterrows():
        predictions += [row['prediction']]*row['numerosity']

    return np.array(predictions).reshape(-1,1)


# def generate_espresso_for_action(df,action,use_integer_payoffs=True):
#     """"Generate the espresso representation from a population loaded into
#     a dataframe for a given action"""

#     clusters = df['label'].unique()
#     clusters.sort()
#     no_clusters = len(clusters)

#     prediction_statistics, df_cluster_statistics = compute_cluster_prediction_statistics(df)

#     df['condition'] = df['condition'].apply(lambda x: x.replace('#','-'))

    
#     df_action = df[df['action']==action].copy()
    

#     if (clusters.min()==-1):
#         raise Exception ("generate_espresso_for_action DOES NOT MANAGE NOISE CLUSTERS")
    
#     df_action['niche'] = df_action['label'].apply(lambda x: str(get_cluster_string(x,no_clusters)))
                
#     input_size = len(df_action['condition'].values[0])
#     output_size = no_clusters

#     if (use_integer_payoffs):
#         # for most cases, this is good enough
#         str_payoffs = " ".join(['p%d'%int(prediction_statistics[i]['average']) for i in clusters])
#     else:
#         str_payoffs = " ".join(['p%.1f'%prediction_statistics[i]['average'] for i in clusters])
    
#     espresso = ".i %d\n.o %d\n"%(input_size, output_size)
#     espresso += ".olb " + str_payoffs + "\n" 

#     # print (".olb " + str_payoffs + "\n" )
#     # print(df)
#     # print("\n")
    
#     for i,row in df_action[['condition','action','niche']].iterrows():
        
#         espresso_input = row['condition']
            
#         espresso_output = row['niche']

#         espresso += espresso_input + " " + espresso_output +"\n"
    
#     espresso += ".e\n" 
#     return espresso

def generate_espresso_for_action(df,action,use_integer_payoffs=True, cluster_label='label'):
    """"Generate the espresso representation from a population loaded into
    a dataframe for a given action"""

    clusters = df[cluster_label].unique()
    clusters.sort()
    no_clusters = len(clusters)

    # print("CLUSTERS ")
    prediction_statistics, df_cluster_statistics = compute_cluster_prediction_statistics(df)

    df['condition'] = df['condition'].apply(lambda x: x.replace('#','-'))

    
    df_action = df[df['action']==action].copy()
    

    if (clusters.min()==-1):
        raise Exception ("generate_espresso_for_action DOES NOT MANAGE NOISE CLUSTERS")
    
    df_action['niche'] = df_action[cluster_label].apply(lambda x: str(get_cluster_string(x,no_clusters)))
                
    input_size = len(df_action['condition'].values[0])
    output_size = no_clusters

    if (use_integer_payoffs):
        # for most cases, this is good enough
        str_payoffs = " ".join(['p%d'%int(prediction_statistics[i]['average']) for i in clusters])
    else:
        str_payoffs = " ".join(['p%.1f'%prediction_statistics[i]['average'] for i in clusters])
    
    espresso = ".i %d\n.o %d\n"%(input_size, output_size)
    espresso += ".olb " + str_payoffs + "\n" 

    # print (".olb " + str_payoffs + "\n" )
    # print(df)
    # print("\n")
    
    for i,row in df_action[['condition','action','niche']].iterrows():
        
        espresso_input = row['condition']
            
        espresso_output = row['niche']

        espresso += espresso_input + " " + espresso_output +"\n"
    
    espresso += ".e\n" 
    return espresso




def get_cluster_string(cluster_index, no_clusters):
    str_cluster = "0"*cluster_index + "1" + "0"*(no_clusters-cluster_index-1)
    return str_cluster


##############################################################
###
### OPTIMAL GENERALIZATION - ESPRESSO FUNCTIONS
### 
##############################################################
def generate_complete_representation(pla_path):
    input_output, no_input_bits, no_output_bits, input_labels, output_labels = load_pla(pla_path)
    all_inputs = generate_all_bitstrings(no_input_bits)
    str_espresso = generate_complete_table(all_inputs, input_output, no_input_bits, no_output_bits,input_labels,output_labels)
    with open(pla_path.replace(".pla","_complete.pla"),"w") as OUTPUT:
        OUTPUT.write(str_espresso)

def generate_complete_representation2(pla_path,all_inputs):
    input_output, no_input_bits, no_output_bits, input_labels, output_labels = load_pla(pla_path)
    all_inputs = generate_all_bitstrings(no_input_bits)
    str_espresso = generate_complete_table(all_inputs, input_output, no_input_bits, no_output_bits,input_labels,output_labels)
    with open(pla_path.replace(".pla","_complete.pla"),"w") as OUTPUT:
        OUTPUT.write(str_espresso)

# def generate_complete_table(all_inputs, input_output, no_input_bits, no_output_bits, input_labels=None, output_labels=None):
#     str_output = ""

#     ### ERRORE CONTROLLA SOLO L'OCCORRENZA NON IL MATCH
#     for input_string in all_inputs:
#         if (input_string in input_output.keys()):
#             str_output += input_string + " " + input_output[input_string] + "\n"
#         else:
#             str_output += input_string + " " + "-"*no_output_bits + "\n"

#     str_espresso = ".i %d\n.o %d\n"%(no_input_bits,no_output_bits)
    
#     if (input_labels is not None):
#         str_espresso += input_labels + "\n"

#     if (output_labels is not None):
#         str_espresso += output_labels + "\n"
    
#     str_espresso += str_output + ".e\n"

#     return str_espresso

def generate_complete_table(all_inputs, input_output, no_input_bits, no_output_bits, input_labels=None, output_labels=None):

    str_output = ""

    # writes all the actual known configurations
    for input_configuration in input_output.keys():
        str_output += input_configuration + " " + input_output[input_configuration] + "\n"

    all_inputs_set = set(all_inputs)
    for input_configuration in input_output.keys():
        matching_inputs = set([input_string for input_string in all_inputs if match(input_configuration,input_string,'-')])
        all_inputs_set = all_inputs_set - matching_inputs

    for input_string in all_inputs_set:
        str_output += input_string + " " + "-"*no_output_bits + "\n"

    str_espresso = ".i %d\n.o %d\n"%(no_input_bits,no_output_bits)

    if (input_labels is not None):
        str_espresso += input_labels + "\n"

    if (output_labels is not None):
        str_espresso += output_labels + "\n"
    
    str_espresso += str_output + ".e\n"

    return str_espresso

    # # for input_string in all_inputs:
    # #     if (sum([match(input_condition,input_string,'-') for input_condition in input_output.keys()]))==0:
    # #         str_output += input_string + " " + "-"*no_output_bits + "\n"

    # str_espresso = ".i %d\n.o %d\n"%(no_input_bits,no_output_bits) + str_output + ".e\n"
    # return str_espresso


def load_pla(pla_path):

    input_label=None

    with open(pla_path,"r") as INPUT:
        pla_text = INPUT.read()

    pla_lines = [input_output.strip() for input_output in pla_text.split("\n") if len(input_output.strip())>0]

    output_labels = [pla_line for pla_line in pla_lines if pla_line[:4]==".olb"]
    if len(output_labels)==1:
        output_label = output_labels[0]
        # print("OUTPUT LABEL ",output_label)
    else:
        output_label = None

    input_labels = [pla_line for pla_line in pla_lines if pla_line[:4]==".ilb"]
    if len(input_labels)==1:
        input_label = input_labels[0]
        # print("OUTPUT LABEL ",input_label)
    else:
        input_label = None

    pla_configurations = [line.split(" ") for line in pla_lines if line[0] in ["0", "1", "-"]]
    
    input_set = set([input_string for input_string,output_string in pla_configurations])
    input_output = {}
    for x,y in pla_configurations:
        input_output[x] = y 

    no_input_bits = len(pla_configurations[0][0])
    no_output_bits = len(pla_configurations[0][1])
    
    return input_output, no_input_bits, no_output_bits, input_label, output_label


### 
### COMPUTE THE STATISTICS
### 
def compute_rolling_statistics(df, columns, window=100, sample=True):

    df_stats = df[df['Problem Type']=='Testing'].copy()

    no_rows = len(df_stats)

    trials = [i for i in range(window, no_rows+1)]

    data_dictionary = {'Trial':trials}

    for column in columns:

        raw_data_series = df_stats[column].squeeze()
        rolling_data = raw_data_series.rolling(window).mean().dropna().values

        data_dictionary[column] = rolling_data

    df = pd.DataFrame(data_dictionary)

    if (sample):
        return df[(df['Trial']%window==0) | (df['Trial'] == df['Trial'].max())].copy()
    else:
        return df

### 
### TRACKING OPTIMIZED POPULATIONS 
###

def compute_cluster_average_prediction(df_population, cluster_variable='label', cluster_prediction_variable='cluster prediction'):

    # create a copy of the dataframe
    df = df_population.copy()

    # compute average prediction for each cluster
    payoff_values_table = df[[cluster_variable,'prediction']].groupby(by=[cluster_variable]).mean()    
    payoff_values_table.reset_index(inplace=True)

    # create a dictionary mapping cluster id to the average prediction
    payoff_values = {}
    for i,row in payoff_values_table.iterrows():
        payoff_values[row[cluster_variable]]=row['prediction']

    # create the column with the average prediction for the cluster
    df[cluster_prediction_variable] = df[cluster_variable].apply(lambda label: payoff_values[label])


    # # compute prediction standard deviation for each cluster
    # payoff_std_values_table = df[[cluster_variable,'prediction']].groupby(by=[cluster_variable]).std()    
    # payoff_std_values_table.reset_index(inplace=True)

    # # create a dictionary mapping cluster id to the average prediction
    # payoff_std_values = {}
    # for i,row in payoff_std_values_table.iterrows():
    #     payoff_std_values[row[cluster_variable]]=row['prediction']

    # df['cluster prediction std'] = df[cluster_variable].apply(lambda label: payoff_std_values[label])

    return df


### THE SAME AS generate_espresso_for_action BUT DOES NOT GENERATE THE LABELS
def generate_espresso_for_action2(df, action, cluster_label='label'):

    actions = df['action'].unique()

    if (len(actions)!=1) or (actions[0]!=action):
        raise Exception ("DATA FRAME CONTAINS MORE THAN ONE ACTION")
    
    clusters = df[cluster_label].unique()
    clusters.sort()
    no_clusters = len(clusters)

    df['condition'] = df['condition'].apply(lambda x: x.replace('#','-'))

    df_action = df.copy()
    

    if (clusters.min()==-1):
        raise Exception ("generate_espresso_for_action DOES NOT MANAGE NOISE CLUSTERS")
    
    df_action['niche'] = df_action[cluster_label].apply(lambda x: str(get_cluster_string(x,no_clusters)))
                
    input_size = len(df_action['condition'].values[0])
    output_size = no_clusters

    str_payoffs = " ".join(['p%d'%i for i in clusters])
    
    espresso = ".i %d\n.o %d\n"%(input_size, output_size)
    espresso += ".olb " + str_payoffs + "\n" 
    
    for i,row in df_action[['condition','action','niche']].iterrows():
        
        espresso_input = row['condition']
            
        espresso_output = row['niche']

        espresso += espresso_input + " " + espresso_output +"\n"
    
    espresso += ".e\n" 
    return espresso