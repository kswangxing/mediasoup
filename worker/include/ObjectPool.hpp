#ifndef __OBJ_POOL_H__
#define __OBJ_POOL_H__

#include <vector>
#include <queue>

template<typename T>
class ObjectPool
{
private:
    std::queue<T*> freePool;
    std::vector<T*> allPool;
    size_t objectSize;
public:
    ObjectPool(size_t size) : objectSize(size) {
    }

    virtual ~ObjectPool() {
        ReleaseObjs();
    }

    template<class... Args>
    T* New(Args&&... args) {
        if (freePool.empty()) {
            AllocateObjs(objectSize);
            fprintf(stderr, "AllocateObjs again!!\n");
        }
            
    
        if (freePool.empty())
            return nullptr;

        T* ret = freePool.front();
        freePool.pop();
        new ( ret )T(std::forward<Args>(args)...);

        return ret;  
    }

    void Delete(T* obj) {
        if (!obj) 
            return;

        obj->~T();
        freePool.push(obj);
    }

private:
    void AllocateObjs(size_t size) {
        if (0 == size)
            return;

        auto pT = reinterpret_cast<T*>(malloc(objectSize * sizeof(T)));
        if(nullptr == pT)
            return;
    
        for(size_t i = 0; i < size; ++i) {
            freePool.push(pT + i);
        }

        allPool.push_back(pT);
    }

    void ReleaseObjs() {
        for (size_t i = 0; i < allPool.size(); ++i)
        {
            free(allPool[i]);
            allPool[i] = nullptr;
        }
        allPool.clear(); 
    }
};

#endif // __OBJ_POOL_H__