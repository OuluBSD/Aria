#ifndef _WebDriver_Shared_h_
#define _WebDriver_Shared_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

struct SharedObjectBase {
private:
	volatile int ref_count_;

public:
	SharedObjectBase() : ref_count_(0) {}
	virtual ~SharedObjectBase() {}
	
	void AddRef() { ref_count_++; }
	void Release() {
		if (--ref_count_ == 0) {
			delete this;
		}
	}
};

template<typename T>
class Shared {
public:
	Shared() : ptr_(nullptr) {}
	
	explicit Shared(T* ptr) : ptr_(ptr) {
		if (ptr_) {
			ptr_->AddRef();
		}
	}
	
	Shared(const Shared& other) : ptr_(other.ptr_) {
		if (ptr_) {
			ptr_->AddRef();
		}
	}
	
	~Shared() {
		if (ptr_) {
			ptr_->Release();
		}
	}
	
	Shared& operator=(const Shared& other) {
		if (this != &other) {
			if (ptr_) {
				ptr_->Release();
			}
			ptr_ = other.ptr_;
			if (ptr_) {
				ptr_->AddRef();
			}
		}
		return *this;
	}
	
	T* operator->() const { return ptr_; }
	T& operator*() const { return *ptr_; }
	T* Get() const { return ptr_; }
	bool IsEmpty() const { return ptr_ == nullptr; }
	
private:
	T* ptr_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif