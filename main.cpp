#include <chrono>
#include <iostream>
#include <string>
#include "eval.h"
#include "minmax.h"
#include "ntuples.h"
#include "position.h"




int main() {
    generate_LUT_data(); //do at prog start
    auto t0=std::chrono::high_resolution_clock::now();
    auto s=0;
    auto pos=othello::Position();
    for (int i=0;i<10000;i++) {
        s+=n_eval(pos);
    }
    auto t1=std::chrono::high_resolution_clock::now();
    std::cout<<std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()<<"\n";
    t0=std::chrono::high_resolution_clock::now();
    for (int i=0;i<10000;i++) {
        s+=othello_engine::eval(pos);
    }
    t1=std::chrono::high_resolution_clock::now();
    std::cout<<std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()<<"\n";

    std::cout<<s<<"\n";
    return 0;
}
