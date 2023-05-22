from torchvision.io import read_image, ImageReadMode
from torchvision.utils import save_image, make_grid
from torchvision.transforms.functional import crop
from torch import zeros
from torch.nn import MSELoss
from os import listdir, path, makedirs
from openpyxl import Workbook
from datetime import datetime
from argparse import ArgumentParser
import matplotlib.pyplot as plt # openCV는 색 반전 이슈, PIL은 속도 이슈

def paintImage(tensor ,paint):
    n = int(args.grid_size)
    crop_size = int(args.image_size) // n
    crops = []
    for i in range(n):
        for j in range(n):
            left = j * crop_size
            upper = i * crop_size
            cropImg = crop(tensor, upper, left, crop_size, crop_size)
            for p in paint:
                row = int(p) // n
                col = int(p) % n
                if i == row and j == col:
                    cropImg[0] = 1
            cropImg[1][0][:] = 1
            cropImg[1][-1][:] = 1
            for k in range(0,crop_size):
                cropImg[1][k][0] = 1
                cropImg[1][k][-1] = 1
            crops.append(cropImg)
    return make_grid(crops,n,0,False)
    
def showFigure(image, paint):
    tensor = read_image(args.data_path + args.network + image,ImageReadMode.RGB_ALPHA).float()
    tensor = tensor.div(255)
    tensor[3] = 0.7
    tensor = paintImage(tensor,paint)
    
    if args.figure_out:
        if not path.exists(args.output_path + time):
            makedirs(args.output_path + time)
        save_image(tensor, args.output_path + time + "/" + image)
    if args.show_image:
        plt.imshow(tensor.permute(1,2,0))
        plt.show()
         
def printResult():
    size = int(args.grid_size)
    for key in result:
        paintList = []
        sort_result = result[key]
        sort_result = list( zip( range(size*size), sort_result ) )
        sort_result.sort(key = lambda x: (x[1]),reverse=True)
        
        print("-------",key.strip(".png"), "-------" ) 
        for i in range(0,min(5,size*size)):
            print("({0}, {1}) : {2:0.5f} ".format( (sort_result[i][0])//size+1, (sort_result[i][0])%size+1,sort_result[i][1]))
            paintList.append(sort_result[i][0])
        print()
        if args.figure_out or args.show_image:
            showFigure(key,paintList)
        
def createResultFile():
    global time
    wb = Workbook()
    ws = wb.active
    head = []
    size = int(args.grid_size)
    # head
    head.append('Distance')
    for i in range(0,size):
        for j in range(0,size):
            head.append('Block(' + str(i+1)+","+str(j+1)+")")
    head.append('All Block')
    ws.append(head)
    
    # body
    for key in result:
        sum = 0
        text = []
        text.append(key.strip('.png'))
        for j in range(0,size*size):
                sum += result[key][j].item()
                text.append( "{:.5f}".format(round(result[key][j].item(),5)) )
        text.append("{:.5f}".format(round(sum/(size*size),5)))
        ws.append(text)
        
    time = datetime.now().isoformat().replace('.','-').replace(':','-')
    wb.save(args.output_path + time + '.xlsx')

def cropImage(image, n):
    crop_size = int(args.image_size) // n
    crops = []
    for i in range(n):
        for j in range(n):
            left = j * crop_size
            upper = i * crop_size
            cropImg = crop(image, upper, left, crop_size, crop_size)
            crops.append(cropImg)
    return crops

def compareMSE():
    global result
    global figure
    result = {}
    mse_loss = MSELoss()
    for truth, network in pairList.items():
        tru_image = read_image(args.data_path + truth, mode=ImageReadMode.RGB)
        net_image = read_image(args.data_path + network, mode=ImageReadMode.RGB)
        tru_crops = cropImage(tru_image, int(args.grid_size))
        net_crops = cropImage(net_image, int(args.grid_size))
        name = network.split(args.network)[1]
        result[name] = []
        for tru, net in zip(tru_crops, net_crops):
            result[name].append(mse_loss(net.float()/255,tru.float()/255))
    
def makePair():
    global pairList
    pairList = {}
    dataList = listdir(args.data_path)
    groundList = []
    networkList = []
    for item in dataList:
        if args.ground_truth in item: 
            groundList.append(item)
        elif args.network in item:
            networkList.append(item)
        else:
            print("[Error] doesn't match : " + item)
            
    for i in groundList:
        splitTMP = i.split(args.ground_truth)[1]
        for j in networkList:
            if splitTMP == j.split(args.network)[1]:
                pairList[i] = j
                break
             
def checkFileSystem():
    if not path.exists(args.data_path):
        makedirs(args.data_path)
    if not path.exists(args.output_path):
        makedirs(args.output_path)

def checkArgument():
    if int(args.image_size) % int(args.grid_size) != 0:
        print("이미지를 Grid로 분할 할 수 없습니다.")
        exit()

def makeParser():
    parser = ArgumentParser(description='Comparision MSE based on grid')
    parser.add_argument('-g', '--grid_size', required=True)
    parser.add_argument('-f', '--figure_out', action='store_true', default=False, help = 'Save Image')
    parser.add_argument('-s', '--show_image', action='store_true', default=False, help = 'Show Image')
    parser.add_argument('-i', '--image_size', default=384)
    parser.add_argument('-d', '--data_path', default='./DATA/')
    parser.add_argument('-o', '--output_path', default='./OUTPUT/')
    parser.add_argument('-t', '--ground_truth', default='hr_recon_')
    parser.add_argument('-n', '--network', default='sr_recon_')
    
    global args
    args = parser.parse_args()
    
if __name__ =="__main__":
    makeParser()
    checkArgument()
    checkFileSystem()
    makePair()
    compareMSE()
    createResultFile()
    printResult()
    