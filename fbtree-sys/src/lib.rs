use cxx::UniquePtr;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("fbtree-sys/include/wrap.h");

        type FbInt;

        fn fbtree_new() -> UniquePtr<FbInt>;

        unsafe fn fbtree_upsert(tree: *mut FbInt, key: u64, value: u64);
        unsafe fn fbtree_update(tree: *mut FbInt, key: u64, value: u64);
        unsafe fn fbtree_lookup(tree: *mut FbInt, key: u64, value: *mut u64) -> bool;
    }
}

pub struct FbTree(UniquePtr<ffi::FbInt>);

unsafe impl Send for FbTree {}
unsafe impl Sync for FbTree {}

impl Default for FbTree {
    fn default() -> Self {
        Self(ffi::fbtree_new())
    }
}

impl FbTree {
    #[inline]
    pub fn upsert(&self, key: u64, value: u64) {
        unsafe {
            ffi::fbtree_upsert(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn update(&self, key: u64, value: u64) {
        unsafe {
            ffi::fbtree_update(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn lookup(&self, key: u64) -> Option<u64> {
        unsafe {
            let mut value = 0u64;
            ffi::fbtree_lookup(self.0.as_mut_ptr(), key, &mut value).then_some(value)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::FbTree;

    #[test]
    fn smoke() {
        let map = FbTree::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            map.upsert(i, i);
        }

        for i in 0..COUNT {
            assert_eq!(map.lookup(i), Some(i));
        }
    }
}
