# CropImage

해당 프로그램은 여러 이미지의 같은 영역을 비교하기 위해서 사용된다.
비교하기 위한 영역은 선택한 영역의 비율을 기반으로 다른 이미지에도 그대로 그려진다.

## Env

Visual Studtio 2019
Release
x64

실행 파일이 있는 PATH에 한국어 지원x (영문만 가능)


## Initial Screen

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/init.png)

초기화면은 다음과 같다.

## Load Images

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/ImageLoad.png)

단축키는 우측의 Option window에서 확인할 수 있다.
이미지를 불러오기 위해서 Image Select 버튼을 누르면 된다.
이미지를 성공적으로 불러올 시 Image Select 버튼 아래의 목록에 파일명이 나온다.

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/SelectImage.png)

그리고 파일명을 누르면 window 창이 뜨며 해당 사진을 확인할 수 있다.

## Select Region & Save Image

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/OneBox.png)

그ꈰ고 영역을 치고 싶은 이미지를 선택하고 ctrl+x를 눌를 경우 window가 닫히고, client에 그려지게 된다. (탈출은 ctrl+c)
그리고 비교하기 위한 영역을 좌클릭+드래그 하여 선택하면된다.
영역이 제대로 설정되었다면 ctrl+s를 눌러 해당 영역을 저장한다.

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/Draw.png)

만약 여러 영역을 선택하고 싶다면 마찬가지로 영역을 선택 후, ctrl+s로 저장하면 된다.

그리고 좌측의 Save를 Image 버튼을 눌러 저장하면 된다.

## Result

파일은 실행 파일이 있는 폴더에 Result 폴더가 생긴다.
(Visual Studio에서 실행 시 (SolutionDir)/x64/Release/Result/(time)/)

![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/Result.png)

그렇다면 선택한 영역에 대한 사진과, 원본 이미지에서 그려진 박스를 확인할 수 있다.


![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/1_3905_hr_recon_-0.0.png)
![](https://github.com/sunbei00/CropImage/blob/CropImageWithImGui/Images/3905_hr_recon_-0.0.png)


## TO DO

Path에 한국어 지원