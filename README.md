# Tiny_Heat_Face_Recognition

TinyML heat image face recognition demo

Edge-Impulse project link :<https://studio.edgeimpulse.com/public/37746/latest/learning/keras/9>

## Getting Started

Clone the repository

`git clone https://github.com/CaptainKAZ/Tiny_Heat_Face_Recognition.git --recurse-submodules`

### Part I Preperation 
**Hardware**: Wio Terminal, Grove - Thermal Imaging Camera (MLX90640) (Both FOV 110° and FOV 55° is acceptable) & SD Card.
**Software**: Arduino & Edge Impluse.
### Part II Theory:
#### Thermal Imaging:
With global outbreak of COVID-19, we are more and more fami familiar with thermal imaging as it is part of the first line defencing the virus. 
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626332870863-10207513-1161-4eb8-b46b-1e1c00676d1a.png#height=1080&id=u6f2957d1&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1080&originWidth=1920&originalType=binary&ratio=1&size=3112934&status=done&style=none&width=1920)
Thermographic, or, thermal imaging cameras usually detect radiation in the long-infrared range of the electromagnetic spectrum (roughly 9,000–14,000 nanometers or 9–14 μm) and produce images of that radiation, called thermograms. Since infrared radiation is emitted by all objects with a temperature above absolute zero according to the black body radiation law, thermography makes it possible to see one's environment with or without visible illumination. The amount of radiation emitted by an object increases with temperature; therefore, thermography allows one to see variations in temperature.
Now we are not going to use the perfessional version of thermal imaging camera with high resolution and accuracy, but using a simple and inexpensive camera sensor module Grove - Thermal Imaging Camera (MLX90640). The module we use has only 32*24 (768) thermal pixels, it is hard for human eye to even recognize object through raw data, but it is optimal for tiny ML magic. With edge impluse and tiny ML we make it possible to recognize people's face.
#### Face Recognition:
As mentioned above, thermal camea is good at captureing the difference in tempreture, our face consist of many muscles, which generates differerent amout of heat and provide enough temperature difference. The muscles shape our face, which also means that we can use ML to classify between people.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626399441106-ff98e9af-c219-4be8-b0ca-26723f5c4dba.png#height=1000&id=uf54184d3&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1000&originWidth=1000&originalType=binary&ratio=1&size=864163&status=done&style=none&width=1000)
Un like visible imaging, thermal imaging can't be cheated by masks or photos, because they do not imitate the distribution of heat,  which provides higher insuarance on security.

#### Flash Firmware
Install [Seeed_Arduino_MLX9064x](https://github.com/Seeed-Studio/Seeed_Arduino_MLX9064x), [Seeed_Arduino_LCD](https://github.com/Seeed-Studio/Seeed_Arduino_LCD), [Seeed_Arduino_FS](https://github.com/Seeed-Studio/Seeed_Arduino_FS) and [Seeed_Arduino_SFUD](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD).
We also need a small lib [TinyJPEG](https://github.com/serge-rgb/TinyJPEG) to encode and save our data.

#### Acquiring the data
Plugin the  Grove - Thermal Imaging Camera (MLX90640) and insert SD card, Flash the sketch, and if everything is OK you will see the thermal image on the screen.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626404215493-d1ec1239-ea74-431f-a5a0-de50db4d31d8.png#height=1080&id=u9433829c&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1080&originWidth=1438&originalType=binary&ratio=1&size=1867457&status=done&style=none&width=1438)
Press A B and C button to capture with different label. Default label is face 0 for A, face 1 for B, environment for C (can be modified on program above). Once you press the button, you will see a number of the same label samples collected and the current label on the screen, and a jpg file is saved to your SD card.
Press A to start "Environment" data acquisition.  Once you press the button A, you will see a number of the same label samples collected and the current label "Environment" on the screen, and a jpg file is saved to your SD card.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626404647962-a4ef240e-6363-4ab9-992e-de5e2d950865.png#height=1080&id=u251ad45d&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1080&originWidth=1438&originalType=binary&ratio=1&size=1648758&status=done&style=none&width=1438)
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626404821606-a6064e34-88c4-47ac-bed3-aec8c5dfee98.png#height=1080&id=u7bead99e&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1080&originWidth=1438&originalType=binary&ratio=1&size=1821356&status=done&style=none&width=1438)
Note that for accurate data acquisition, the reflash rate of the sensor is relatively slow. You should hold the sensor until the image is stable before start capturing.
Wrong example:
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626405207074-18888669-f4d1-4aed-be8c-3414188fdd81.png#height=1438&id=u69f0a198&margin=%5Bobject%20Object%5D&name=image.png&originHeight=1438&originWidth=1080&originalType=binary&ratio=1&size=1650891&status=done&style=none&width=1080)
#### Uploading the data
After collecting about 200 samples for each labels, we can turn off the power and plug the SD card to our compueter.
If everything goes right, there should be jpeg files on your disk.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626405581400-6a9ce5d0-a19b-4e44-8d90-85f2e578dfd0.png#height=169&id=u8c1714ae&margin=%5Bobject%20Object%5D&name=image.png&originHeight=169&originWidth=132&originalType=binary&ratio=1&size=4160&status=done&style=none&width=132)
Face image is collected in grey scale.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626405659712-83d7f687-e3ea-427d-bbde-19f52cdb81ee.png#height=24&id=u5f31275b&margin=%5Bobject%20Object%5D&name=image.png&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1560&status=done&style=none&width=32)![lhy.13.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793427-2c59347b-1265-4349-9f4d-f3f6d7f1fb1b.jpeg#height=24&id=uc84cd8d5&margin=%5Bobject%20Object%5D&name=lhy.13.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1366&status=done&style=none&width=32)![lhy.14.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793419-3380d989-6317-408f-a4da-d1a3c3df2254.jpeg#height=24&id=uc1b96f9a&margin=%5Bobject%20Object%5D&name=lhy.14.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1374&status=done&style=none&width=32)![lhy.15.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793430-41b16649-22b9-495b-b6ff-dfc37a45a341.jpeg#height=24&id=u2dc2d9ef&margin=%5Bobject%20Object%5D&name=lhy.15.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1378&status=done&style=none&width=32)![lhy.16.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793420-cfd1d8f7-94d2-4fd9-8bdd-053e60eb9723.jpeg#height=24&id=u21c74cc3&margin=%5Bobject%20Object%5D&name=lhy.16.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1370&status=done&style=none&width=32)![lhy.17.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793426-42645d9e-ab50-4335-bce6-a8871b8f9e3e.jpeg#height=24&id=u93e912cd&margin=%5Bobject%20Object%5D&name=lhy.17.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1353&status=done&style=none&width=32)![lhy.18.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793782-51596f02-86e4-400c-a72b-409a77b4628d.jpeg#height=24&id=u6738e355&margin=%5Bobject%20Object%5D&name=lhy.18.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1365&status=done&style=none&width=32)![lhy.19.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793830-7a3a70b3-5cfd-4174-ae64-5701c223d06c.jpeg#height=24&id=ud1cafb0e&margin=%5Bobject%20Object%5D&name=lhy.19.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1366&status=done&style=none&width=32)![lhy.20.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793901-5b4c78b8-b9e9-4da3-b18e-1a67de605800.jpeg#height=24&id=u0e2c8c47&margin=%5Bobject%20Object%5D&name=lhy.20.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1369&status=done&style=none&width=32)![lhy.21.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405793930-39b62a5b-a213-4b89-90bb-54ba7bdfeaa5.jpeg#height=24&id=ubbfcb450&margin=%5Bobject%20Object%5D&name=lhy.21.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1386&status=done&style=none&width=32)![lhy.22.jpg](https://cdn.nlark.com/yuque/0/2021/jpeg/21842784/1626405794034-c0a50b1c-098a-4408-9266-f1c0cab977e0.jpeg#height=24&id=u0edbf2d6&margin=%5Bobject%20Object%5D&name=lhy.22.jpg&originHeight=24&originWidth=32&originalType=binary&ratio=1&size=1378&status=done&style=none&width=32)
Open [Edge Impulse](https://www.edgeimpulse.com/) and create a new project. Upload your files in data acquisition tag.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626406014051-3ac895e7-11f7-49cf-a9bc-28277a61b08f.png#height=90&id=ue83482c4&margin=%5Bobject%20Object%5D&name=image.png&originHeight=90&originWidth=203&originalType=binary&ratio=1&size=6400&status=done&style=none&width=203)
Select all your sample and upload to edge impulse
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626406090086-d997b5e0-18a2-4ca3-a8bb-4805b8f86f55.png#height=669&id=u86f84c57&margin=%5Bobject%20Object%5D&name=image.png&originHeight=669&originWidth=738&originalType=binary&ratio=1&size=40178&status=done&style=none&width=738)
After that you should see your data on the dash board.
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626406173343-ff0235db-bf97-4509-9a4c-8bfc0b4d2a1d.png#height=472&id=u186e3ad6&margin=%5Bobject%20Object%5D&name=image.png&originHeight=472&originWidth=1555&originalType=binary&ratio=1&size=188354&status=done&style=none&width=1555)
### Part IV Model Design & Training
Impulse I created.
Use image data input, raw data DSP and Kearas NN classifier. 
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626406423775-e8e35f6d-d8a3-4849-a794-50c02156a942.png#height=470&id=u3a32bb28&margin=%5Bobject%20Object%5D&name=image.png&originHeight=470&originWidth=1580&originalType=binary&ratio=1&size=306444&status=done&style=none&width=1580)
The transfer learning EI provided is not suitable for such a small image and limited computing resource.
CNN architecture:
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626407117762-9ffcb9fc-1790-4901-9634-6bd0814d1339.png#height=432&id=u56fa2a9c&margin=%5Bobject%20Object%5D&name=image.png&originHeight=432&originWidth=759&originalType=binary&ratio=1&size=52196&status=done&style=none&width=759)
The traing parameters:
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626407162931-21331635-7c36-4b74-9aac-11672cd60ace.png#height=235&id=u54b86479&margin=%5Bobject%20Object%5D&name=image.png&originHeight=235&originWidth=752&originalType=binary&ratio=1&size=14921&status=done&style=none&width=752)
And the result:
![image.png](https://cdn.nlark.com/yuque/0/2021/png/21842784/1626407203537-89621609-eb56-41db-b86a-4af2a7bf3673.png#height=724&id=u3c59c3db&margin=%5Bobject%20Object%5D&name=image.png&originHeight=724&originWidth=726&originalType=binary&ratio=1&size=89463&status=done&style=none&width=726)
100% val accuracy is not the best result but a little overfitting. Maybe it is bacause I set the model too wide instead of deep, and model just "memorize" the input and output.
### Part V Model Deploy
Import the lib from the Edge-impulse and flash the Data Collection Sketch
