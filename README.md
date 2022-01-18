# Flash-DBSim Simulation System

[Flash-DBSim](https://github.com/yuesong-feng/Flash-DBSim) is a simulation tool for evaluating Flash-based database algorithms used for flash-based researches, we'd like to make Flash-DBSim useful for all researchers in their researches and experiments.

Original paper by Su Xuan: [Flash-DBSim: A simulation tool for evaluating Flash-based database algorithms](https://ieeexplore.ieee.org/document/5234967)

## how to use
```bash 
make
./run
```
`src/flashdbsim_i.h` is the only file exposed for users, you should only include this file in your program. 

`class BMgr` in file `BufferAlgorithm.h` is a virtual base class, all buffer algorithms should inherit this class. 

Full details: Please refer to [Flash-DBSim](http://kdelab.ustc.edu.cn/flash-dbsim/index_en.html)

## Copyright
[Flash-DBSim](http://kdelab.ustc.edu.cn/flash-dbsim/index_en.html) was originally developed in 2008 by [KDELab of USTC](http://kdelab.ustc.edu.cn/index.html). And was redeveloped by me in 2022. The main changes are as follows：
- A major rewrite/reorganization of the code.
- Greatly simplify code structure, get rid of DLL and Visual Studio framework. Now the simulator doesn't depend on any Windows library. 
- Generalize to almost all operating systems including Linux, Windows, macOS, FreeBSD. etc. 
- Migrate to modern C++11, but is downward compatible with older versions.
- Change coding method to UTF-8 to fix the disordered code problem.
- Numerous other bugs were fixed.

Flash-DBSim Simulation System. Copyright © 2008-2009, KDELab@USTC. All rights reserved.

Flash-DBSim Simulation System. Copyright © 2022, FENG-Yuesong@HKU (ysfeng@connect.hku.hk). All rights reserved.

