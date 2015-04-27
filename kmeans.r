wssplot <- function(data, nc=15, seed=1234){
			    wss <- (nrow(data)-1)*sum(apply(data,2,var))
               for (i in 2:nc){
                    set.seed(seed)
                    wss[i] <- sum(kmeans(data, centers=i)$withinss)}
                plot(1:nc, wss, type="b", xlab="Number of Clusters",
                     ylab="Within groups sum of squares")}


x<-read.csv("inputCluster.csv",header=TRUE)
a<-x$STG
b<-x$SCG
c<-x$STR
d<-x$LPR
e<-x$PEG
final_class<-x$UNS
z<-data.frame(a,b,c,d,e)

df<-scale(z[-1])
wssplot(df)

km<-kmeans(z,4,15) 
print(km)
y<-data.frame(a,b,c,d,e,km$cluster)
plot(km$cluster)


