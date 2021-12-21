import sys
        
def minIndex(lst): #Recieves a list of integers, returns index of minimum value
    minIndex = 0;

    if (len(lst) == 1):
        return 0

    currMin = lst[0]
    
    for i in range(1,len(lst)):
        if (lst[i] < currMin):
            minIndex = i
            currMin = lst[i]

    return minIndex


def subtractVectors (v1,v2): #Recieves vectors v1,v2 ,  returns v1-v2
    return [v1[i]-v2[i] for i in range(0,len(v1))]

def addVectors (v1,v2): #Recieves vectors v1,v2 ,  returns v1+v2
    return [v1[i]+v2[i] for i in range(0,len(v1))]


def squareVector(v): #Recieves a vector v, returns ||v||^2
    s = 0
    for cord in v:
        s += cord**2
    #s = s**2
    return s

def divVector(v,a): #Recieves a vector v and scalar a, returns v/a
    return [v[i]/a for i in range(0,len(v))]

def decimPlacesVector(v): #Recieves a vector v, return it with all coordiantes with 4 places after decimal dot
    return ['%.4f'%v[i] for i in range(0,len(v))]


def kmeans(datapoints_input, K, max_iter): #Recieves a list of N d-dimensional vectors, represented as lists
    #TODO: check if possible that N=0
    N = len(datapoints_input)
    d = len(datapoints_input[0])
    datapoints = [[datapoints_input[i],None] for i in range(0,N)]
    
    clusters =[[datapoints[i][0],datapoints[i][0],1] for i in range(0,K)] #Intiailize clusters, a list where each element is a list ([centroid, curSum, cur|Si|]
    #TODO: maybe do copy?
    
    for i in range(0,K): #assign to each datapoint in x1,...,xK the index of to it's cluster
        datapoints[i][1] = i        

    changedCentroid = True
    
    while ((changedCentroid == True) and (max_iter > 0)):
        #print("iter"+str(max_iter))
        max_iter-=1
        changedCentroid = False
        for i in range(0,N):
            xi = datapoints[i]
            index = minIndex([squareVector(subtractVectors(xi[0],clusters[j][0])) for j in  range(0,K)]) #find index of xi's destined cluster
            if ((xi[1] == None) or (index != xi[1])): #xi's cluster should change
                if (xi[1] != None): #if xi had a previous cluster
                    clusters[xi[1]][1] = subtractVectors(clusters[xi[1]][1] , xi[0]) #subtract xi from curSum in xi's previous cluster
                    clusters[xi[1]][2] -= 1 #subtract 1 from xi's previous cluster's size
                    
                clusters[index][1] = addVectors(clusters[index][1],xi[0]) #add xi to new cluster's curSum
                clusters[index][2] +=1 #add 1 to xi's new cluster's size
                xi[1] = index
                

        #update all centroids:
        for j in range(0,K):
            prev = clusters[j][0]
            if (clusters[j][2] != 0):
                clusters[j][0] = divVector(clusters[j][1],clusters[j][2]) #divVector isn't in-place, returns new
            if (clusters[j][0] != prev):
                changedCentroid = True

    toReturn = [decimPlacesVector(clusters[i][0]) for i in range(0,K)]
    #print (str(clusters[0][2])+", "+str(clusters[1][2])+" ,"+str(clusters[2][2]))

    return toReturn

            

#####MAIN#####

cmdinput = sys.argv

assert (len(cmdinput) > 1), "Not enough arguments! Should at least specify argument K"

assert(str.isnumeric(cmdinput[1]) == True), "Invalid argument: K should be an integer!"

K=int(cmdinput[1])
assert (K > 0), "Invalid value for K! Should be > 0"

if (len(cmdinput) > 2):
    assert(str.isnumeric(cmdinput[2]) == True), "Invalid argument: max_iter should be an integer!"
    if (cmdinput[2].isnumeric()):
        max_iter=int(cmdinput[2])
        assert(max_iter > 0), "Invalid value for max_iter! Should be > 0"
else:
    max_iter=200

assert (len(cmdinput) <= 3), "Too many arguments! Should specify at most K and max_iter"

datapoints_input=[]

try:
    while (True):
        readtext=input().split(",")
        datapoint= [float(cord) for cord in readtext]
        datapoints_input.append(datapoint)
except EOFError:
    assert (len(datapoints_input) > K), "K should be < N !"
    toPrint=kmeans(datapoints_input,K,max_iter)
    for centroid in toPrint:
        string = str(centroid)
        string = string[1:len(string)-1].replace(" ","")
        string = string.replace("'","")
        print(string)

    
            
