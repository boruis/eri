/*
 *  observer.h
 *  eri
 *
 *  Created by exe on 2011/3/10.
 *  Copyright 2011 cobbler. All rights reserved.
 *
 */

#ifndef ERI_OBSERVER_H
#define ERI_OBSERVER_H

#include <cstddef>

namespace ERI
{

template <class T>
class Observer
{
public:
	virtual ~Observer() {}
	
	virtual void OnNotified(T& info) = 0;
};

template <class T>
class Subject
{
public:
	virtual ~Subject() {}
	
	void AddObserver(Observer<T>* observer)
	{
		int observer_num = observers_.size();
		for (int i = 0; i < observer_num; ++i)
		{
			if (observers_[i] == observer)
				return;
		}
		
		observers_.push_back(observer);
	}
	
	void RemoveObserver(Observer<T>* observer)
	{
		int observer_num = observers_.size();
		for (int i = 0; i < observer_num; ++i)
		{
			if (observers_[i] == observer)
			{
				if (i < observer_num - 1)
					observers_[i] = observers_[observer_num - 1];
				
				observers_.pop_back();
				return;
			}
		}
	}
	
	void Notify(T& info)
	{
		size_t observer_num = observers_.size();
		for (int i = 0; i < observer_num; ++i)
		{
			observers_[i]->OnNotified(info);
		}
	}
	
	bool HaveObserver()
	{
		return !observers_.empty();
	}
	
private:
	std::vector<Observer<T>*>	observers_;
};

}

#endif // ERI_OBSERVER_H
