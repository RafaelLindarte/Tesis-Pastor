from obspy import read
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import subplots, plot
import csv
import time, os, shutil

def dowloandEvents(start,end,mag,Plt=True,save=True,station='BAR2'):
    """ 
    Esta función permite descargar miniSeed con las formas de ondas desde el SGC.
    los parámetros para la búsqueda (y como usarlos) son los siguientes:
    start: '2020-04-01T06:47:58',end: '2020-04-01T06:50:38',station: 'BAR2',Plt: True, save : True
    Se debe verificar que exita el evento en la página del servicio geológico colombiano:
        https://www.sgc.gov.co/sismos
    Con decargar dos minutos es más que suficiente para observar los frentes de ondas (P y S)
    """
    st = read('http://sismo.sgc.gov.co:8080/fdsnws/dataselect/1/query?starttime='+start+'&endtime='+end+'&network=CM&sta='+station+'&cha=HH?&loc=00&format=miniseed&nodata=404')
    st0 = (st[0][0:10000])
    st1 = (st[1][0:10000])
    st2 = (st[2][0:10000])
    Pathh = './validationData/' 
    RUTA = Pathh+"MAG"+str(mag)+'_'+start[0:13]
    os.makedirs(RUTA,exist_ok=True)
    if Plt:
        figsize=(16,9)
        fig,ax = plt.subplots(nrows=3, ncols=1, figsize=figsize, facecolor='w', edgecolor='k',
                                squeeze=False,sharex=True)
        ax = ax.ravel() 
        ymax1 = (np.abs(st[1].max()) / 1_000) * 1_000
        ymin1 = ymax1*-1
        ax[0].plot(st[1][0:10000],color='k',linestyle='-',linewidth=0.6, label='HHN')
        ax[0].yaxis.set_ticks(np.arange(ymin1,ymax1,ymax1*0.20))
        ax[0].set_title("Estación_"+station+"_MAG"+str(mag)+'_'+start,fontsize=20,weight='semibold')
        ax[0].legend(fontsize = 15,loc='best')
        ax[1].plot(st[2][0:10000],color='k',linestyle='-',linewidth=0.6, label='HHZ')
        ymax1 = (np.abs(st[2].max()) / 1_000) * 1_000
        ymin1 = ymax1*-1
        ax[1].yaxis.set_ticks(np.arange(ymin1,ymax1,ymax1*0.20))
        ax[1].legend(fontsize = 15,loc='best')
        ax[2].plot(st[0][0:10000],color='k',linestyle='-',linewidth=0.6,label='HHE')
        ymax1 = (np.abs(st[0].max()) / 1_000) * 1_000
        ymin1 = ymax1*-1
        ax[2].yaxis.set_ticks(np.arange(ymin1,ymax1,ymax1*0.20))
        ax[2].legend(fontsize = 15,loc='best')
        fig.savefig(RUTA+'/'+station+'_MAG'+str(mag)+'_'+start[0:13]+'.png',dpi=500)
        #plt.show()
        plt.close()
    if save:
        with open(RUTA+'/'+station+'_MAG'+str(mag)+'_'+start[0:13]+'_HHN'+".csv", 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(st1)
        with open(RUTA+'/'+station+'_MAG'+str(mag)+'_'+start[0:13]+'_HHZ'+".csv", 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(st2)
        with open(RUTA+'/'+station+'_MAG'+str(mag)+'_'+start[0:13]+'_HHE'+".csv", 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(st0)
    return st0,st1,st2

# ---------------------------Pruebas Calidad-------------------------
B43 = dowloandEvents('2020-05-31T15:02:50','2020-05-31T15:04:50',2.1)# -> Prueba Calidad
B47 = dowloandEvents('2020-05-16T00:38:45','2020-05-16T00:40:45',2.2)# -> Prueba Calidad
B48 = dowloandEvents('2020-05-15T14:31:22','2020-05-15T14:33:22',2.2)# -> Prueba Calidad
B41 = dowloandEvents('2020-05-18T06:52:34','2020-05-18T06:54:34',2.1)# -> Prueba Calidad
B17 = dowloandEvents('2020-06-23T17:53:04','2020-06-23T17:55:04',2.8)# -> Prueba Calidad
# ---------------------------Set De Datos----------------------------
B37 = dowloandEvents('2020-06-15T02:39:48','2020-06-15T02:41:48',2.0)
B38 = dowloandEvents('2020-06-18T18:15:13','2020-06-18T18:17:13',2.0)
B39 = dowloandEvents('2020-05-14T10:07:11','2020-05-14T10:09:11',2.0)
B40 = dowloandEvents('2020-06-10T13:39:47','2020-06-10T13:41:47',2.0)
B42 = dowloandEvents('2020-06-27T19:36:15','2020-06-27T19:38:15',2.0)
B03 = dowloandEvents('2022-05-31T10:23:07','2022-06-01T21:50:33',2,1)
B04 = dowloandEvents('2020-06-29T17:36:16','2020-06-29T17:38:16',2.1)
B44 = dowloandEvents('2020-06-15T04:22:10','2020-06-15T04:24:10',2.1)
B01 = dowloandEvents('2020-04-01T06:47:58','2020-04-01T06:50:38',2.2)
B45 = dowloandEvents('2020-06-24T04:17:59','2020-06-24T04:19:59',2.2)
B46 = dowloandEvents('2020-06-10T15:05:58','2020-06-10T15:07:58',2.2)
B05 = dowloandEvents('2020-05-31T12:41:13','2020-05-31T12:43:13',2.2)
B06 = dowloandEvents('2020-06-06T00:27:15','2020-06-06T00:29:15',2.2)
B07 = dowloandEvents('2020-06-09T23:54:12','2020-06-09T23:56:12',2.3)
B08 = dowloandEvents('2020-05-01T16:52:18','2020-05-01T16:54:18',2.3)
B49 = dowloandEvents('2020-05-17T13:45:33','2020-05-17T13:47:33',2.3)
B50 = dowloandEvents('2020-06-20T17:59:33','2020-06-20T18:01:33',2.3)
B09 = dowloandEvents('2020-05-28T13:05:32','2020-05-28T13:07:32',2.4)
B10 = dowloandEvents('2020-06-04T03:06:12','2020-06-04T03:08:12',2.4)
B11 = dowloandEvents('2020-06-27T10:55:30','2020-06-27T10:57:30',2.5)
B12 = dowloandEvents('2020-06-06T20:46:58','2020-06-06T20:48:58',2.5)
B13 = dowloandEvents('2020-06-23T09:41:07','2020-06-23T09:43:07',2.6)
B14 = dowloandEvents('2020-05-25T05:38:47','2020-05-25T05:40:47',2.6)
B15 = dowloandEvents('2020-05-09T08:15:17','2020-05-09T08:17:17',2.7)
B16 = dowloandEvents('2020-05-16T21:38:15','2020-05-16T21:40:15',2.7)
B18 = dowloandEvents('2020-05-16T22:54:53','2020-05-16T22:56:53',2.8)
B19 = dowloandEvents('2020-07-02T04:18:02','2020-07-02T04:20:02',2.9)
B20 = dowloandEvents('2020-06-20T16:01:51','2020-06-20T16:03:51',2.9)
B21 = dowloandEvents('2020-06-22T20:39:41','2020-06-22T20:41:41',3.0)
B22 = dowloandEvents('2020-05-23T03:50:37','2020-05-23T03:52:37',3.0)
B23 = dowloandEvents('2020-05-01T16:36:33','2020-05-01T16:38:33',3.1)
B24 = dowloandEvents('2020-06-02T20:13:50','2020-06-02T20:15:50',3.1)
B25 = dowloandEvents('2020-05-18T02:58:07','2020-05-18T03:00:07',3.2)
B26 = dowloandEvents('2020-06-11T11:53:24','2020-06-11T11:55:24',3.3)
B27 = dowloandEvents('2020-05-18T02:39:39','2020-05-18T02:41:39',3.4)
B28 = dowloandEvents('2020-05-14T18:40:55','2020-05-14T18:42:55',3.4)
B29 = dowloandEvents('2020-05-07T01:26:25','2020-05-07T01:28:25',3.5)
B30 = dowloandEvents('2020-05-12T17:37:29','2020-05-12T17:39:29',3.5)
B31 = dowloandEvents('2020-06-08T23:19:30','2020-06-08T23:21:30',3.6)
B32 = dowloandEvents('2020-05-06T04:42:14','2020-05-06T04:44:14',3.6)
B33 = dowloandEvents('2020-05-30T18:07:16','2020-05-30T18:09:16',3.7)
B34 = dowloandEvents('2020-05-05T16:28:12','2020-05-05T16:30:12',3.8)
B35 = dowloandEvents('2020-05-19T15:04:29','2020-05-19T15:06:29',4.2)
B36 = dowloandEvents('2020-05-05T16:28:12','2020-05-05T16:30:12',4.5)
B02 = dowloandEvents('2022-05-16T11:25:56','2022-05-16T11:28:56',4.5)