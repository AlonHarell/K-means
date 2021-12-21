#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct datapoint datapoint;
typedef struct cluster cluster;

void kmeans(void*,void*,int,int,int,int);
int minIndex(double[],int);
double* subtractVectors(double[],double[],int);
void subtractVectors_inplace(double[],double[],int);
void addVectors_inplace(double[],double[],int);
double squareVector(double[],int);
void divVector(double[],int,int);
double* copyVector(double[], int);
int equalVectors(double[], double[], int);
int getVectorSize(char*);
double* inputToVector(char*, int);
int isNumber (char*, int);


struct datapoint /**struct represents datapoint and it's cluster**/
{
    double* vector;
    int cluster_id;
};

struct cluster
{
    double* centroid;
    double* sum; /** sum of all datapoints in cluster**/
    int count; /** number of datapoints in cluster**/
};


int main(int argc, char **argv)
{
    /**Reminder: might need to use lots of "free" commands right after the argmin calculation**/
    int N,K,d,i,j,size,max_iter,KisNumber,max_iterisNumber;
    char *input;
    datapoint *datapoints_input;
    cluster *clusters;
    max_iter = 200;
    N=0;
    d=-1;
    size=1;

    datapoints_input = (datapoint*)calloc(1,sizeof(datapoint)); /**array of datapoints**/

    /**arguments reading and validation:**/
    if (datapoints_input == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(datapoints_input != NULL);
    }

    if (argc <= 1)
    {
        printf("Not enough arguments! Must at least specify argument K!\n");
        assert(argc > 1);
    }

    KisNumber = isNumber(argv[1],(int)strlen(argv[1]));
    if (KisNumber == 0)
    {
        printf("Invalid argument: K should be an integer! \n");
        assert(KisNumber == 1);
    }
    K=atoi(argv[1]);
    if (K <=0)
    {
        printf("Invalid value for K, should be >0 !\n");
        assert (K > 0);
    }

    if (argc > 2)
    {
        max_iterisNumber = isNumber(argv[2],(int)strlen(argv[2]));
        if (max_iterisNumber == 0)
        {
            printf("Invalid argument: max_iter should be an integer! \n");
            assert(max_iterisNumber == 1);
        }

        max_iter = atoi(argv[2]);
        if (max_iter <= 0)
        {
            printf("Invalid value for max_iter, should be >0 !\n");
            assert(max_iter > 0);
        }
    }

    if (argc > 3)
    {
        printf("Too many arguments! Should specify at most K and max_iter\n");
        assert(argc <= 3);
    }

    input = (char*)calloc(1000,sizeof(char)); /**will be used to read input file**/
    if (input == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(input != NULL);
    }

    while(scanf("%s",input) != EOF)
    {
        N++;

        if (d == -1)
        {
            d = getVectorSize(input); /**happens once, gets dimension of vectors**/
        }
        datapoints_input[N-1].cluster_id = -1;
        datapoints_input[N-1].vector = inputToVector(input,d); /**add current line's vector to the datapoints array**/

        if(size == N) /**doubling array size: dynamic array size increase, O(N) amortized**/
        {
            size = size * 2;
            datapoints_input = (datapoint*)realloc(datapoints_input,size*sizeof(datapoint));
            if (datapoints_input == NULL)
            {
                printf("Memory allocation failed!\n");
                assert(datapoints_input != NULL);
            }
        }

        free(input);
        input = (char*)calloc(1000,sizeof(char)); /**reallocate input, prevent old memory by filling with 0s**/
        if (input == NULL)
        {
            printf("Memory allocation failed!\n");
            assert(input != NULL);
        }
        
    }

    free(input);
    
    if (K >= N)
    {
        printf("K must be <N !\n");
        assert (K < N);
    }

    datapoints_input = (datapoint*)realloc(datapoints_input,N*sizeof(datapoint)); /**resize array to size of N**/
    if (datapoints_input == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(datapoints_input != NULL);
    }

    clusters = (cluster*)calloc(K,sizeof(cluster)); /**create K-sized array of clusters**/
    if (clusters == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(clusters != NULL);
    }

    kmeans(clusters,datapoints_input, N, d, K, max_iter); /**actual kmeans algorithm, updates centroids of array clusters**/

    /**PRINTING:**/
    for (i=0; i<K; i++)
    {
        for (j=0; j<d; j++)
        {
            printf("%.4f",clusters[i].centroid[j]);
            if (j < d-1)
            {
                printf(",");
            }
        }

        if (i < K-1)
        {
            printf("\n");
        }
    }

    for (i=0; i<K; i++)
    {
        free(clusters[i].centroid);
        free(clusters[i].sum);
    }
    free(clusters);

    return 0;
}

int getVectorSize(char* input) /**used for first vector, returns dimension**/
{
    int i=0, d = 1;
    while ((i<1000) && (input[i] != 0))
    {
        if (input[i] == ',')
        {
            d++;
        }

        i++;
    }
    return d;
}

double* inputToVector(char* input, int d) /**converts string divided by commas to a vector, array of doubles**/
{
    char *substring;
    double *vector;
    int elem,i,j,n;
    i=0;
    n=0;
    elem = 0;

    vector = (double*)calloc(d,sizeof(double));
    if (vector == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(vector != NULL);
    }

    while (elem < d) /**d is known, scanning the string for d elements**/
    {
        i = n;
        while ((input[n] != ',') && (input[n] != 0)) /**get bounds (in characters) of current element (will be from i to n)**/
        {
            n++;
        }

        substring = (char*)calloc(n,sizeof(char));
        if (substring == NULL)
        {
            printf("Memory allocation failed!\n");
            assert(substring != NULL);
        }

        for (j=0; j<n; j++)
        {
            substring[j]=input[i];
            i++;
        }

        vector[elem] = atof(substring); /**convert current element to double**/
        free(substring);
        substring = NULL;
        
        n++;
        elem++;
            
    }

    return vector;
        

}

void kmeans(void* clusters_input, void* datapoints_input, int N, int d, int K, int max_iter)
{
    int i,j, changedCentroid,index_of_min;
    double squareValue, *vectorsDif, *squaresArr, *newcentroid;
    datapoint *datapoints = (datapoint*)datapoints_input; /**get the array of datapoints, assumed to be of size N**/
    cluster *clusters = (cluster*)clusters_input; /**create K-sized array of clusters**/
    for (i=0; i<N; i++)
    {
        datapoints[i].cluster_id = -1; /**update datapoint's cluster as none (-1)**/
        if (i<K) /**if i<K then datapoint should be assigned to i-th cluster**/
        {
            clusters[i].centroid = copyVector(datapoints[i].vector,d);
            clusters[i].sum = copyVector(datapoints[i].vector,d);
            clusters[i].count = 1;
            
            datapoints[i].cluster_id = i;
        }
    }

    changedCentroid = 1; /**keeps track if centroid changed during iteration**/

    squaresArr = (double*)calloc(K,sizeof(double));
    if (squaresArr == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(squaresArr != NULL);
    }

    while ((changedCentroid == 1) && (max_iter > 0))
    {
        max_iter -=1;
        changedCentroid = 0;

        for (i=0; i<N; i++) /**iterate over centroids**/
        {
            
            for (j=0; j<K; j++) /**create array of ||xi-uj||^2  **/
            {
                vectorsDif = subtractVectors(datapoints[i].vector,clusters[j].centroid,d);
                squareValue = squareVector(vectorsDif,d);
                free(vectorsDif);
                squaresArr[j]=squareValue;
            }
            index_of_min = minIndex(squaresArr,K); /**find minimum, meaning closest centroid to xi**/

            if ((datapoints[i].cluster_id == -1) || (datapoints[i].cluster_id != index_of_min))  /**xi's destined cluster should change**/
            {
                if (datapoints[i].cluster_id != -1) /**xi had a previous cluster**/
                {
                    subtractVectors_inplace(clusters[datapoints[i].cluster_id].sum,datapoints[i].vector,d); /**subtract xi from prev's sum**/
                    clusters[datapoints[i].cluster_id].count -=1;
                }

                addVectors_inplace(clusters[index_of_min].sum,datapoints[i].vector,d); /**add xi to new cluster's sum**/
                clusters[index_of_min].count +=1;
                datapoints[i].cluster_id = index_of_min;
            }
       
        }

        /**update all centroids:**/
        for (i=0; i<K; i++)
        {
            
            newcentroid = copyVector(clusters[i].sum,d);
            if (clusters[i].count != 0)
            {
                divVector(newcentroid,clusters[i].count,d); /**new centroid = sum divided by count**/
            }
            
            if (equalVectors(newcentroid,clusters[i].centroid,d) == 0) /**not equal, so should change centroid**/
            {
                free(clusters[i].centroid);
                clusters[i].centroid = newcentroid;
                changedCentroid = 1;
            }
            else /**equal, no need to change**/
            {
                free(newcentroid);
            }

        }


    }
    free(squaresArr);
    

    for (i = 0; i<N; i++) /**free datapoints array, wont need it anymore**/
    {
        free(datapoints[i].vector);
    }

    free(datapoints);
    
}


int minIndex(double v[] , int K) /**gets array of doubles of size K, returns index of min element**/
{
    int i, minIndex;
    double currentMin;
    minIndex = 0;
    currentMin = v[0];
    if (K == 1)
    {
        return 0;
    }

    for (i=1; i<K; i++)
    {
        if (v[i] < currentMin)
        {
            minIndex = i;
            currentMin = v[i];
        }
    }

    return minIndex;
}

double* subtractVectors(double v1[], double v2[], int d) /**returns vector v3 = v1 - v2**/
{
    int i;
    double *v3;
    v3 = (double*)calloc(d,sizeof(double));
    if (v3 == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(v3 != NULL);
    }
    for (i=0; i<d; i++)
    {
        v3[i]=v1[i]-v2[i];
    }

    return v3;
}

void subtractVectors_inplace(double v1[], double v2[], int d) /**foreach i,  v1[i]=v1[i]-v2[i]*/
{
    int i;
    for (i=0; i<d; i++)
    {
        v1[i]=v1[i]-v2[i];
    }
}

void addVectors_inplace(double v1[], double v2[], int d) /**foreach i,  v1[i]=v1[i]+v2[i]*/
{
    int i;
    for (i=0; i<d; i++)
    {
        v1[i]=v1[i]+v2[i];
    }
}

double squareVector(double v[], int d) /**returns ||v||^2 **/
{
    int i;
    double s = 0;
    for (i = 0; i<d; i++)
    {
        s += (v[i]*v[i]);
    }

    return s;
}

void divVector(double v[], int a, int d) /**returns vector v divided by scalar a**/
{
    int i;
    for (i=0; i<d; i++)
    {
        v[i] = v[i]/((double)a);
    }
}

double* copyVector(double v[], int d) /**returns a copy of vector v**/
{
    int i;
    double* vcopy;
    vcopy = (double*)calloc(d,sizeof(double));
    if (vcopy == NULL)
    {
        printf("Memory allocation failed!\n");
        assert(vcopy != NULL);
    }
    for (i=0; i<d; i++)
    {
        vcopy[i] = v[i];
    }

    return vcopy;
}

int equalVectors(double v1[], double v2[], int d) /**checks if vectors v1,v2 are equal**/
{
    int i;
    for (i=0; i<d; i++)
    {
        if (v1[i] != v2[i])
        {
            return 0;
        }
    }
    return 1;
}

int isNumber (char* str, int len) /** return 1 if str is a number, 0 otherwise**/
{
    int i;
    for (i = 0; i<len; i++)
    {
        if ((str[i] > 57) || (str[i] < 48))
        {
            return 0;
        }
    }
    return 1;
}



