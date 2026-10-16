# Crypto_50877_Group2
Run cpp script through WSL: Ubuntu

installing valgrind:
sudo apt update
sudo apt install -y valgrind

installing libhydrogen directly from github:
https://github.com/jedisct1/libhydrogen/wiki/Installation
-download hydrogen.c and hydrogen.h and /impl directory into the repository

If red squiggly still appears in hydrogen.h, hydrogen.c then ensure C/C++ Edit Configurations (JSON) includePath is set to the below:
```
"includePath": [
                "${workspaceFolder}/**"
            ],
```

