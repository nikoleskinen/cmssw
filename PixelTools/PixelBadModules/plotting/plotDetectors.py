from collections import namedtuple
import re
from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt
from itertools import cycle, product
from matplotlib.pyplot import cm

def spanpat(var,b='[]'):
    F = r'([-+]?\d*\.\d+|[-+]?\d+)'
    if b ==  ('[]'):
        return r'\b' + var + r'\s*:\s*\[\s*'+F+r'\s*,\s*'+F+r'\s*\]'
    elif b == ('<>'):
        return r'\b' + var + r'\s*:\s*<\s*'+F+r'\s*,\s*'+F+r'\s*>'
    else:
        pass

def intpat(var):
    F = r'([-+]?\d*\.\d+|\d+)'
    return r'\b' + var + r'\s*:\s*\[\s*'+F+r'\s*\]'

def readDetectors(filename):
    with open(filename) as f:
        D = f.readlines()
    detsBarrel = []
    detsEndcap = []
    DetectorBarrel = namedtuple('DetectorBarrel','id subdetid layer ladder module phiSpan zSpan rSpan')
    DetectorEndcap = namedtuple('DetectorEndcap','id subdetid disk blade panel phiSpan zSpan rSpan')
    for line in D:
        idm = re.search(intpat('id'),line)
        subdetidm = re.search(intpat('subdetid'),line)
        layerm = re.search(intpat('layer'),line)
        ladderm = re.search(intpat('ladder'),line)
        modulem = re.search(intpat('module'),line)

        diskm = re.search(intpat('disk'),line)
        bladem = re.search(intpat('blade'),line)
        panelm = re.search(intpat('panel'),line)

        phim = re.search(spanpat('phi'),line)
        zm = re.search(spanpat('z'),line)
        rm = re.search(spanpat('r'),line)
        if idm and subdetidm and layerm:
            B = DetectorBarrel(
                    int(idm.group(1)),
                    int(subdetidm.group(1)),
                    int(layerm.group(1)),
                    int(ladderm.group(1)),
                    int(modulem.group(1)),
                    (float(phim.group(1)),float(phim.group(2))),
                    (float(zm.group(1)),  float(zm.group(2))),
                    (float(rm.group(1)),  float(rm.group(2))),
                    )
            detsBarrel.append(B)
        if idm and subdetidm and diskm:
            B = DetectorEndcap(
                    int(idm.group(1)),
                    int(subdetidm.group(1)),
                    int(diskm.group(1)),
                    int(bladem.group(1)),
                    int(panelm.group(1)),
                    (float(phim.group(1)),float(phim.group(2))),
                    (float(zm.group(1)),  float(zm.group(2))),
                    (float(rm.group(1)),  float(rm.group(2))),
                    )
            detsEndcap.append(B)
    return detsBarrel,detsEndcap
def readClusterSpans(filename):
    with open(filename) as f:
        D = f.readlines()
    clusterSpans = []
    ClusterSpan = namedtuple('ClusterSpanEndcap','subdetid phiSpan zSpan rSpan')
    for line in D:
        subdetidm = re.search(intpat('subdetid'),line)
        phim = re.search(spanpat('phi','<>'),line)
        zm = re.search(spanpat('z','<>'),line)
        rm = re.search(spanpat('r','<>'),line)
        if subdetidm :
            S = ClusterSpan(
                    int(subdetidm.group(1)),
                    (float(phim.group(1)),float(phim.group(2))),
                    (float(zm.group(1)),  float(zm.group(2))),
                    (float(rm.group(1)),  float(rm.group(2))),
                    )
            clusterSpans.append(S)
    return clusterSpans
def plotdetectorsBarrel(dets,fig=None,ax=None,**kwargs):
    if not fig:
        fig=plt.figure()
    if not ax:
        ax =plt.axes(projection='3d')
    for det in dets:
        if det.phiSpan[0]-det.phiSpan[1] < np.pi:
            phi = np.linspace(det.phiSpan[0],det.phiSpan[1]+2*np.pi,2)
        else:
            phi = np.linspace(det.phiSpan[0],det.phiSpan[1],2)
        z = np.linspace(det.zSpan[0],det.zSpan[1],2)
        # closer surface
        for ind in (0,1):
            x = det.rSpan[ind]*np.cos(phi)
            y = det.rSpan[ind]*np.sin(phi)
            X,Z = np.meshgrid(x,z)
            Y = np.tile(y,(len(z),1))
            ax.plot_surface(X,Z,Y,**kwargs)
    ax.set_xlabel('X')
    ax.set_ylabel('Z')
    ax.set_zlabel('Y')
    return fig,ax
def plotdetectorsEndcap(dets,fig=None,ax=None,**kwargs):
    if not fig:
        fig=plt.figure()
    if not ax:
        ax =plt.axes(projection='3d')
    for det in dets:
        if det.phiSpan[0]-det.phiSpan[1] < np.pi:
            phi = np.linspace(det.phiSpan[0],det.phiSpan[1]+2*np.pi,2)
        else:
            phi = np.linspace(det.phiSpan[0],det.phiSpan[1],2)
        r = np.linspace(det.rSpan[0],det.rSpan[1],2)
        R,PHI = np.meshgrid(r,phi)
        X = R*np.cos(PHI)
        Y = R*np.sin(PHI)
        for ind in (0,1):
            Z = X*0+det.zSpan[ind]
            ax.plot_surface(X,Z,Y,**kwargs)
    return fig,ax
def phiZRToXYZ(point):
    return (point[2]*np.cos(point[0]),point[2]*np.sin(point[0]),point[1])
def plotClusterSpans(spans,fig=None,ax=None,**kwargs):
    if not fig:
        fig=plt.figure()
    if not ax:
        ax=plt.axes(projection='3d')
    colors = iter( cm.rainbow(np.linspace(0,1,len(spans))) )
    for span in spans:
        if span.subdetid == 2:
            continue
        color = next(colors)
        for i,j,k in product([0,1],[0,1],[0,1]):
            P = (span.phiSpan[i],span.zSpan[j],span.rSpan[k])
            P = phiZRToXYZ(P)
            x = [0, P[0]]
            y = [0, P[1]]
            z = [0, P[2]]
            ax.plot(x,z,y,color=color,**kwargs)
    return fig,ax 

    pass
if __name__ == "__main__":
    B,E = readDetectors('DETECTORS')
    BB,BE = readDetectors('BADDETECTORS')
    spans = readClusterSpans('CLUSTERSPANS')
    fig,ax = plotClusterSpans(spans)
    #fig,ax = plotdetectorsBarrel(B,None,None,color='k',alpha=0.02)
    #fig,ax = plotdetectorsEndcap(E,fig,ax,color='k',alpha=0.02)
    fig,ax = plotdetectorsBarrel(BB,fig,ax,color='r',alpha=0.2)
    #fig,ax = plotdetectorsEndcap(BE,fig,ax,color='r',alpha=0.2)
    ax.grid(False)
    fig.patch.set_facecolor('white')
    ax.set_facecolor('white')
    ax.w_xaxis.set_pane_color((1.0, 1.0, 1.0, 1.0))
    ax.w_yaxis.set_pane_color((1.0, 1.0, 1.0, 1.0))
    ax.w_zaxis.set_pane_color((1.0, 1.0, 1.0, 1.0))
    ax.set_axis_off()
    ax.view_init(90,0)
    fig.show()
    input()
