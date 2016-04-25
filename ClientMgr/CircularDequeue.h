#pragma once

#include <mutex>
#include <vector>

template < class T >
class CircularDequeue {
private:
	std::mutex mutex_data;
	std::vector< T > list;
	int _capacity, _size;
	int first, last;

public:
	CircularDequeue( int const capacity  ) {
		list.resize( capacity );
		_capacity = capacity;
		_size = 0;
		first = 0; last = 0;
	}

	~CircularDequeue() { }

	int size() { return _size; }
	int capacity() { return _capacity; }

	bool contains( T const & other ) {
		for( int i = first; i <= last; i++ ) {
			if( list[ i % _capacity ] == other )
				return true;
		}
		return false;
	}

	bool resize( int const capacity ) {
		std::lock_guard< std::mutex > lock( mutex_data );
		if( _size == 0 ) {
			list.resize( capacity );
			_capacity = capacity;
			list.shrink_to_fit();
			_size = 0;
			first = 0; last = 0;
			return true;
		}
		return false;
	}

	bool push_front( T const & data ) {
		std::lock_guard< std::mutex > lock( mutex_data );
		if( contains( data ) )
			return false;

		if( _size == 0 ) {
			first = 0; last = 0;
			list[ first ] = data;

			_size++;
			return true;
		}
		else if( _size < _capacity ) {
			first--; _size++;
			if( first < 0 ) {
				first += _capacity;
				last += _capacity;
			}
			list[ first % _capacity ] = data;
			return true;
		}
		return false;
	}

	bool push_back( T const & data ) {
		std::lock_guard< std::mutex > lock( mutex_data );
		if( contains( data ) )
			return false;

		if( _size == 0 ) {
			first = 0; last = 0;
			list[ first ] = data;
			_size++;
			return true;
		}
		else if( _size < _capacity ) {
			last++;	_size++;
			list[ last % _capacity ] = data;
			return true;
		}
		return false;
	}

	bool try_push_front( T const & data ) {
		if( mutex_data.try_lock() ) {
			std::lock_guard< std::mutex > lock( mutex_data, std::adopt_lock );
			if( _size == 0 ) {
				first = 0; last = 0;
				list[ first ] = data;
				_size++;
				return true;
			}
			else if( _size < _capacity ) {
				first--; _size++;
				if( first < 0 ) {
					first += _capacity;
					last += _capacity;
				}
				list[ first % _capacity ] = data;
				return true;
			}
			return false;
		}
		return false;
	}

	bool try_push_back( T const & data ) {
		if( mutex_data.try_lock() ) {
			std::lock_guard< std::mutex > lock( mutex_data, std::adopt_lock );
			if( _size == 0 ) {
				first = 0; last = 0;
				list[ first ] = data;
				_size++;
				return true;
			}
			else if( _size < _capacity ) {
				last++;
				list[ last % _capacity ] = data;
				_size++;
				return true;
			}
			return false;
		}
		return false;
	}

	bool pop_front( T & data ) {
		std::lock_guard< std::mutex > lock( mutex_data );
		if( _size > 0 ) {
			data = list[ first % _capacity ];
			first++; _size--;
			if( first >= _capacity ) {
				first -= _capacity;
				last -= _capacity;
			}
			return true;
		}
		return false;
	}

	bool pop_back( T & data ) {
		std::lock_guard< std::mutex > lock( mutex_data );
		if( _size > 0 ) {
			data = list[ last % _capacity ];
			last--; _size--;
			return true;
		}
		return false;
	}

	bool try_pop_front( T const & data ) {

	}

	bool try_pop_back( T const & data ) {

	}
};

