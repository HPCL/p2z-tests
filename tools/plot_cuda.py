import numpy as np
from scipy import loadtxt
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.backends.backend_pdf import PdfPages

z = open('output_tuner/cudaoutput_nvcc_cuda.txt')
events = []
for line in z.readlines():
  col = line.rstrip().split(' ')
  events.append({"fullname": col[0], "compiler": col[1], "mode": col[2], "stream":int(col[3]) , "block":int(col[4]), "threadx":int(col[5]) ,"thready":int(col[6]), "i":int(col[7]), "events":int(col[8]), "compute":float(col[9]), "computepertrk":float(col[10]), "region":float(col[11]),"regionpertrk":float(col[11])/float(col[8]), "mem":float(col[12]),"mempertrk":float(col[12])/float(col[8]), "setup":float(col[13]),"setuppertrk":float(col[13])/float(col[8])})

df = pd.DataFrame(events)
dfx = df.groupby(['fullname','stream','block','threadx','thready','events']).mean()
dfx = dfx.reset_index()
corrMatrix = dfx.corr()
print(corrMatrix)

#print(df)
#print(dfx)

import seaborn as sns
sns.heatmap(corrMatrix, annot=True)
plt.show()
#sub = dfx[dfx['threadx']==1]
#test = dfx['thready'].unique()
def getsub(dfx,n1,n2,n3,n4,xname,yname,xscale):
  pp = PdfPages('cuda_outplots/%s_vs_%s_by_%s.pdf'%(yname,xname,n4))
  for x1 in dfx[n1].unique():
    df1 = dfx[dfx[n1]==x1]
    for x2 in df1[n2].unique():
      df2 = df1[df1[n2]==x2]
      for x3 in df2[n3].unique():
        df3 = df2[df2[n3]==x3]
        fig, ax = plt.subplots()
        for x4 in df3[n4].unique():
          df4 = df3[df3[n4]==x4]
          print(df4)
          df4.plot(x=xname,y=yname,style='.-', ax=ax,label=x4)
        #print(dfx['thready'].unique())
        ax.set_title("%s;%s=%s;%s=%s"%(x1,n2,x2,n3,x3))
        ax.set_xlabel(xname)
        ax.set_ylabel(yname)
        ax.set_xscale(xscale)
        ax.set_xticks(dfx[xname].unique())
        ax.set_xticklabels(dfx[xname].unique())
        ax.legend(title=n4)
        plt.ticklabel_format(axis='y',style="sci")
        #plt.show()
        pp.savefig()
          #for x5 in df4[n5].unique():
          #  df5 = df4[df4[n5]==x5]
  pp.close()    

#getsub(dfx,'fullname','stream','thready','threadx','block','regionpertrk','linear')
#getsub(dfx,'fullname','stream','thready','threadx','block','computepertrk','linear')
#getsub(dfx,'fullname','stream','thready','threadx','block','mempertrk','linear')
#getsub(dfx,'fullname','stream','block','threadx','thready','regionpertrk','log')
#getsub(dfx,'fullname','stream','block','threadx','thready','computepertrk','log')
#getsub(dfx,'fullname','stream','block','threadx','thready','mempertrk','log')
#getsub(dfx,'fullname','stream','block','thready','threadx','regionpertrk','log')
#getsub(dfx,'fullname','stream','block','thready','threadx','computepertrk','log')
#getsub(dfx,'fullname','stream','block','thready','threadx','mempertrk','log')
#getsub(dfx,'fullname','threadx','thready','stream','block','computepertrk','linear')
#getsub(dfx,'fullname','threadx','thready','stream','block','regionpertrk','linear')
#getsub(dfx,'fullname','threadx','thready','stream','block','mempertrk','linear')
#getsub(dfx,'fullname','threadx','thready','block','stream','computepertrk','linear')
#getsub(dfx,'fullname','threadx','thready','block','stream','regionpertrk','linear')
#getsub(dfx,'fullname','threadx','thready','block','stream','mempertrk','linear')
