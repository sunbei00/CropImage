import cv2
import numpy as np
import os
import openpyxl as op 
import sys
from datetime import datetime

#sys.stdout = open("output.txt",'w')

path = './Data/'
resultPath = './Result/'
block = 0 # 인자 값 넣는 것으로 변경 

np.set_printoptions(threshold=np.inf, linewidth=np.inf)
CheckList = {}
dirList = []
Result = {}

def setup():
    if not os.path.exists(path):
        os.makedirs(path)
    if not os.path.exists(resultPath):
        os.makedirs(resultPath)
    if len(sys.argv) != 2:
        print("not argument")
        sys.exit()
    global block
    block = int(sys.argv[1])
    
    dirList = os.listdir(path)
    for folder in dirList:
        fileList = os.listdir(path + folder)
        if len(fileList) == 2:
            CheckList[folder] = [fileList[0], fileList[1]]
        else:
            print("\'" + folder + "\' does not contain 2 picture", sep="\n")


def MSE(img1 : cv2.Mat, img2 : cv2.Mat, key):
    # np.divide(np.array(img2),255) # openCV mat는 음수를 자동으로 0으로 바꾸기 때문에 numpy 사용
    #array1 = np.array(img1) 
    #array2 = np.array(img2) 
    #array = array1 - array2
    
    array = np.divide(img1,255.0) - np.divide(img2,255.0) # 정규화 (0~1)
    
    result = np.power(array,2) 
    
    start_col = [0 for i in range(0,block)]
    end_col = [0 for i in range(0,block)]
    start_row = [0 for i in range(0,block)]
    end_row = [0 for i in range(0,block)]
    MSE_result = [[float(0) for i in range(0,block)] for j in range(0,block)]
    channel_depth = result.shape[2]
    # if channel_depth == 4: # 알파 값 제거, 다만 imread 기본 속성 값이 rgb 3으로 고정되어있어서 안 쓰임
    #     channel_depth = 3 
    
    # image divid block
    for i in range(0,block): # shape[0] : row, shape[1] : col
        start_col[i] = int(i*result.shape[1]/block)
        end_col[i] = int((i+1)*result.shape[1]/block)
        start_row[i] = int(i*result.shape[0]/block)
        end_row[i] = int((i+1)*result.shape[0]/block)
        if i == block-1:
            end_col[i] = result.shape[1]
            end_row[i] = result.shape[0] 
    
    # calculate
    b = 0
    for j in range(0,block):
        for row in range(start_row[j],end_row[j]):
            a = 0
            for i in range(0,block):
                for col in range(start_col[i],end_col[i]):
                    for channel in range(0,channel_depth): # RGB 각각 계산
                        MSE_result[b][a] += result[row,col,channel]
                a += 1
        b += 1 
    
    # 넓이만큼 나누기
    for i in range(0,block):
        for j in range(0,block):
            MSE_result[i][j] = MSE_result[i][j] / float((end_col[j]-start_col[j])*(end_row[i]-start_row[i])) / float(channel_depth) # rgb로 총 최댓 값 3이 나올 수 있음 따라서 channel_depth로 나눔
    
    Result[key] = MSE_result
    #print(MSE_result ,sep='\n')
    
def compare():
    for key in CheckList:
        img1 = cv2.imread(path + key + '/' + CheckList[key][0])
        img2 = cv2.imread(path + key + '/' + CheckList[key][1])
        MSE(img1,img2,key)
              
              
def makeFile():
    wb = op.Workbook()
    ws : op.Sheet = wb.active
    text = []
    # head
    text.append('Folder')
    for i in range(0,block):
        for j in range(0,block):
            text.append('Block(' + str(i+1)+","+str(j+1)+")")
    text.append('All Block')
    ws.append(text)
    
    # body
    for key in Result:
        sum = 0
        text = []
        text.append(key)
        for j in range(0,block):
            for i in range(0,block):
                sum += Result[key][j][i]
                text.append(Result[key][j][i])
        text.append(sum/(block*block))
        ws.append(text)
        
    time = datetime.now().isoformat().replace('.','-').replace(':','-')
    wb.save(resultPath + time + '.xlsx')
    
def main():
    setup()
    compare()
    makeFile()
    print('finish..')
            
    
if __name__ =="__main__":
    main()
    
    # 문제 1. 인터넷에서 다운받은 사진이랑 그림판으로 수정한 것이랑 차이가 발생 : 추측1. 그림판에서 자동으로 보정해줘서
    #           ->> 해결 : BMP 확장자를 안쓰면 jpg같은 압축 성질 때문에 이상한 값이 들어옴.
    #           ->> jpg 써도 (n. * e-07) 수준으로 나오는 것임.
    # 문제 2. alpha 값이 들어오면 그것도 MSE해버리는 문제 -> 4이면 3으로 바꾸기. 나중에 추가
    #           ->> deprh 값이 4면 3으로 변경으로 해결 그런데 순서대로 rgba 인지 확인 한 번 해봐야 함.
    #           ->> imread 기본 값이 rgb 3개만 되게끔 입력받고 있음 따라서 신경x
    #           ->> 흑백 이미지도 rgb 값을 각각 받아옴 하지만 동일한 값으로 받아오기 때문에 depth만큼 빼는 것도 문제 없음
    # 문제 3. 255로 나누면서 실수형 연산 값이 문제가 발생
    #           ->> np.where로 해결
    # 문제 4. 이상한 곳에 값이 들어가는 문제
    #           ->> 문제 1과 동일한 문제
    # 엑셀 형태로 출력 만들거 ->> 완료