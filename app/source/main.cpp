#include "mdb_proxy.h"
#include <thread>
#include <vector>

std::vector<std::thread> threads;

int main(){
    int cpu_id = sched_getcpu();
    printf("cpu_id: %d\n", cpu_id);

    MMdbProxy::CMdbProxyImpl mdbProxy;
    mdbProxy.set_runStatue(true);

    for(int i = 0; i < 5; i++){//5个线程accept阻塞接受，并将建立的数据收发套接字push到队列
        mdbProxy.start_serv(2000);
    }

    for(int i = 0; i < 10; i++){//10个线程处理accept压入队列的套接字，在该套接字上进行数据收发
        threads.push_back(std::thread(&MMdbProxy::CMdbProxyImpl::deal_client, &mdbProxy));
    }

    for(auto & pair : threads){
        pair.join();
    }
    
    return 0;
}