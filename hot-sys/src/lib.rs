use cxx::UniquePtr;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("hot-sys/include/wrap.h");

        type HOTTree;

        fn hottree_new() -> UniquePtr<HOTTree>;

        unsafe fn hottree_upsert(tree: *mut HOTTree, key: u64, value: u32) -> bool;
        unsafe fn hottree_search(tree: *mut HOTTree, key: u64, value: *mut u32) -> bool;
    }
}

pub struct HotTree(UniquePtr<ffi::HOTTree>);

unsafe impl Send for HotTree {}
unsafe impl Sync for HotTree {}

impl Default for HotTree {
    fn default() -> Self {
        Self(ffi::hottree_new())
    }
}

impl HotTree {
    #[inline]
    pub fn upsert(&self, key: u64, value: u32) -> bool {
        unsafe {
            ffi::hottree_upsert(self.0.as_mut_ptr(), key, value)
        }
    }

    #[inline]
    pub fn search(&self, key: u64, value: &mut u32) -> bool {
        unsafe {
            ffi::hottree_search(self.0.as_mut_ptr(), key, &mut *value)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::HotTree;

    #[test]
    fn smoke() {
        let tree = HotTree::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            tree.upsert(i, i as u32);
        }

        for i in 0..COUNT {
            let mut val = 0;
            tree.search(i, &mut val);
            assert_eq!(val, i as u32);
        }
    }
}
