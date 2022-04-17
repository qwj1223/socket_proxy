#include "mdb_proxy.h"
#include <thread>
#include <vector>

std::vector<std::thread> threads;

int main(){
    MMdbProxy::CMdbProxyImpl mdbProxy;
    mdbProxy.set_runStatue(true);
    mdbProxy.start_serv(2000);

    for(int i = 0; i < 10; i++){        
        threads.push_back(std::thread(&MMdbProxy::CMdbProxyImpl::deal_client, &mdbProxy));
    }

    for(auto & pair : threads){
        pair.join();
    }
    
    return 0;
}