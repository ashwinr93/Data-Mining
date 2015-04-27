x<-read.csv("clusterInput.csv",header=TRUE)
a<-x$STG
b<-x$SCG
c<-x$STR
d<-x$LPR
e<-x$PEG
z<-data.frame(a,b,c,d,e)
d <- dist(z, method = "euclidean")
fit <- hclust(d, method="ward") 
plot(fit,labels=FALSE,hang=-1) 
groups <- cutree(fit, k=4) 
rect.hclust(fit, k=4, border="red")