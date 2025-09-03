# A Memetic Algorithm for Vehicle Routing with Simultaneous Pickup and Delivery and Time Windows

This repository is the implementation in C++ for the paper [A memetic algorithm for vehicle routing with simultaneous pickup and delivery and time windows](https://leria-info.univ-angers.fr/%7Ejinkao.hao/papers/Lei-HaoIEEETEVC2024.pdf) by Zhenyu Lei, Jin-Kao Hao.

The Vehicle Routing Problem with Simultaneous Pickup and Delivery and Time Windows (VRPSPDTW) has a number of real-world applications, especially in reverse logistics. In this work, we propose an effective memetic algorithm that integrates a lightweight feasible and infeasible route descent search and a learning-based adaptive route-inheritance crossover to solve this complex problem. We evaluate the effectiveness of the proposed algorithm on the set of 65 popular benchmark instances as well as 20 real-world large-scale benchmark instances. We provide a comprehensive analysis to better understand the design and performance of the proposed algorithm.

## Overview

In this repository, we have:

- `solutions/` folder provides the solutions for benchmark instances
- `include/` and `src/` folders contain the source code of the algorithm.
- `CMakelists.txt` is the cmake configuration file.

## Run the code

The well-known benchmark instances [WC](https://oz.nthu.edu.tw/~d933810/test.htm) and the large-scale real-world benchmark instances [JD](https://github.com/senshineL/VRPenstein) were tested in our work. Please download and configure the instances before running the code.

In addition, please configure other necessary directories and parameters in `include/Config.hpp` file.

And then compile the code by the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

Then you can run directly the executable binary with default parameters set in `include/Config.hpp` by the following command:

```bash
./MA-FIRD
```

You can also run the code with your own parameters by the following command:

```bash
./MA-FIRD --dataset_dir <dataset dir> --output_dir <output dir> -D <datasets> -I <instance> -S <seed> -c <dispatch cost> -v <unit cost> -t <max iterations> -e <max  running time> -p <patience> -z <population size> -r <init ratio> -y <fitness coefficient> -u <adjust factor>
```

## Citation

If it is helpful for your research, please cite our paper:

```bibtex
@article{lei2024memetic,
  title={A memetic algorithm for vehicle routing with simultaneous pickup and delivery and time windows},
  author={Lei, Zhenyu and Hao, Jin-Kao},
  journal={IEEE Transactions on Evolutionary Computation},
  year={2024},
  publisher={IEEE}
}
```

