#Handbook

##坐标轴

* 储存的3D matrix一定是人的正面沿着x轴，从（0，0，0）开始到（x，0，0）；人的侧面是沿着y轴，从（0，0，0）开始到（0，y，0）；垂直方向是从（0，0，0）到（0，0，z）。这个x，y，z是指在3D matrix中的index，还没有考虑每个voxel的长短。
* 只允许从正面，右面和上面看（y<0, x>length, z>height）

## classes

### Voxel

* attenuation：这个value代表在这个3D的voxel


TODO:
only support front side and rotate matrix to get other angles
