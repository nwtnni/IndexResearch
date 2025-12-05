use cxx::UniquePtr;
use std::ffi::CString;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("hot-sys/include/wrap.h");

        type HOTTreeU64;
        type HOTTreeString;

        fn hottree_u64_new() -> UniquePtr<HOTTreeU64>;

        unsafe fn hottree_u64_upsert(tree: *mut HOTTreeU64, key: u64, value: u64) -> bool;
        unsafe fn hottree_u64_search(tree: *mut HOTTreeU64, key: u64, value: *mut u64) -> bool;

        fn hottree_string_new() -> UniquePtr<HOTTreeString>;

        unsafe fn hottree_string_upsert(
            tree: *mut HOTTreeString,
            key: *const c_char,
            keylen: usize,
            value: u64,
        ) -> bool;

        unsafe fn hottree_string_search(
            tree: *mut HOTTreeString,
            key: *const c_char,
            value: *mut u64,
        ) -> bool;
    }
}

// u64 impls
pub struct HotTreeU64(UniquePtr<ffi::HOTTreeU64>);

unsafe impl Send for HotTreeU64 {}
unsafe impl Sync for HotTreeU64 {}

impl Default for HotTreeU64 {
    fn default() -> Self {
        Self(ffi::hottree_u64_new())
    }
}

impl HotTreeU64 {
    #[inline]
    pub fn upsert_u64(&self, key: u64, value: u64) {
        unsafe {
            let _ = ffi::hottree_u64_upsert(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn search_u64(&self, key: u64, value: &mut u64) -> bool {
        unsafe {
            ffi::hottree_u64_search(self.0.as_mut_ptr(), key, value)
        }
    }
}

// string impls

pub struct HotTreeString(UniquePtr<ffi::HOTTreeString>);

unsafe impl Send for HotTreeString {}
unsafe impl Sync for HotTreeString {}

impl Default for HotTreeString {
    fn default() -> Self {
        Self(ffi::hottree_string_new())
    }
}

impl HotTreeString {
    #[inline]
    pub fn upsert(&self, key: &str, value: u64) {
        let cstr = CString::new(key).expect("key must not contain interior NUL bytes");
        unsafe {
            let _ = ffi::hottree_string_upsert(
                self.0.as_mut_ptr(),
                cstr.as_ptr(),
                key.len(),
                value,
            );
        }
    }

    #[inline]
    pub fn search(&self, key: &str) -> Option<u64> {
        let cstr = CString::new(key).expect("key must not contain interior NUL bytes");
        unsafe {
            let mut value = 0u64;
            if ffi::hottree_string_search(self.0.as_mut_ptr(), cstr.as_ptr(), &mut value) {
                Some(value)
            } else {
                None
            }
        }
    }
}
#[cfg(test)]
mod tests {
    use super::{HotTreeString, HotTreeU64};

    #[test]
    fn smoke_u64() {
        let tree = HotTreeU64::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            tree.upsert_u64(i, i);
        }

        for i in 0..COUNT {
            let mut val = 0;
            assert!(tree.search_u64(i, &mut val));
            assert_eq!(val, i);
        }
    }

    #[test]
    fn smoke_string() {
        let tree = HotTreeString::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            let key = format!("key{i:06}");
            tree.upsert(&key, i);
        }

        for i in 0..COUNT {
            let key = format!("key{i:06}");
            assert_eq!(tree.search(&key), Some(i));
        }
    }
}

