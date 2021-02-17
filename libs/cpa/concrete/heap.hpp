#ifndef _HEAP__
#define _HEAP__

#include <memory>

#include "support/types.hpp"
#include "hash/hashing.hpp"
#include "util/array.hpp"
#include "support/pointer.hpp"
#include "except.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {
	  enum class EntryState {
		InUse = 2,
		Freed = 4
	  };
	  struct HeapEntry {
		HeapEntry (MiniMC::uint64_t size) : state(EntryState::InUse),
											size(size),
											memory(new MiniMC::uint8_t[size]) {
		  std::fill  (memory.get(),memory.get()+size,0);
		}
											
		HeapEntry& write (const MiniMC::Util::Array& arr,  MiniMC::uint64_t offset) {
		  assert(state == EntryState::InUse);
		  if ( size+offset <= this->size ) {
			//Copy the existing memory
			MiniMC::uint8_t* nmem = new MiniMC::uint8_t[this->size];
			std::copy (memory.get(),memory.get()+this->size,nmem);
			std::copy (arr.get_direct_access(),arr.get_direct_access()+arr.getSize(),nmem+offset);
			memory.reset(nmem);
			return *this;
		  }
		  else 
			throw BufferOverflow();
		}
				
		void read (MiniMC::Util::Array& arr, MiniMC::uint64_t offset) {
		  if ( arr.getSize()+offset <= this->size ) {
			arr.set_block (0,arr.getSize(),memory.get()+offset);
		  }
		  else 
			throw BufferOverflow ();
		}

		auto hash () const {
		  return MiniMC::Hash::Hash (memory.get(),size,static_cast<MiniMC::Hash::seed_t> (state));
		}

		void setState (EntryState state) {
		  this->state=state;
		}
		
		EntryState state;
		MiniMC::uint64_t size;
		std::shared_ptr<MiniMC::uint8_t[]> memory;
		
	  };
	  

	}
  }
}


namespace std {
  template<>
  struct hash<MiniMC::CPA::Concrete::HeapEntry> {
	auto operator() (const MiniMC::CPA::Concrete::HeapEntry& entry)  {
	  return entry.hash ();
	}
  };
}

namespace MiniMC {
  namespace CPA {
	namespace Concrete {	  
	  class Heap {
	  public:
		Heap () {}

		MiniMC::pointer_t allocate (MiniMC::uint64_t size) {
		  auto pointer = MiniMC::Support::makeHeapPointer (entries.size(),0);
		  entries.emplace_back (size);
		  return pointer;
		}

		void free (MiniMC::pointer_t pointer) {
		  auto base = MiniMC::Support::getBase(pointer);
		  auto offset = MiniMC::Support::getBase(pointer);
	
		  if (base < entries.size() &&
				offset == 0) {
			  entries.at(base).setState (EntryState::Freed);
			}
		  else {
			throw InvalidFree ();
		  }
		}

		void read (MiniMC::Util::Array& arr, MiniMC::pointer_t pointer) {
		  auto base = MiniMC::Support::getBase(pointer);
		  auto offset = MiniMC::Support::getBase(pointer);
		  if (base < entries.size()) {
			entries.at(base).read (arr,offset);
		  }

		  else {
			throw BufferOverflow ();
		  }
		}

		void write (const MiniMC::Util::Array& arr, MiniMC::pointer_t pointer) {
		  auto base = MiniMC::Support::getBase(pointer);
		  auto offset = MiniMC::Support::getBase(pointer);
		  if (base < entries.size()) {
			entries.at(base).write (arr,offset);
		  }
		  
		  else {
			throw BufferOverflow ();
		  }
		}
		
		auto hash () const {
		  MiniMC::Hash::seed_t seed = 0;
		  for (auto& entryt : entries) {
			MiniMC::Hash::hash_combine (seed,entryt);
		  }
		  return seed;
		}
		
	  private:
		std::vector<HeapEntry> entries;
	  };
		
	}
  }
}

namespace std {

  template<>
  struct hash<MiniMC::CPA::Concrete::Heap> {
	auto operator() (const MiniMC::CPA::Concrete::Heap& heap)  {
	  return heap.hash ();
	}
  };
}


#endif
