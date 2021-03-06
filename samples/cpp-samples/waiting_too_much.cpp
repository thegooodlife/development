#include <iostream>
#include <deque>
#include <thread>         // std::thread
#include <mutex>          // std::mutex
using namespace std;

deque<int> q;
mutex mu;

void function_1() {  
    int count = 10;
    while(count>0){
        unique_lock<mutex> locker(mu);
        q.push_front(count);
        locker.unlock();
        this_thread::sleep_for(chrono::seconds(1));
        --count;
    }
    cout <<"done"<< endl;
}

void function_2(){
    int data = 0;
    while(data !=1)
    {
        unique_lock<mutex> locker(mu);
        if(!q.empty()){
            data = q.back();
            q.pop_back();
            locker.unlock();
            cout << data << endl;
        } else {
            
            locker.unlock();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}

int main()
{
    thread t1(function_1);
    thread t2(function_2);
    t1.join();
    t2.join();
    return 0;
}
