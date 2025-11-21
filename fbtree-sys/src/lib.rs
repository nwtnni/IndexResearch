use cxx::UniquePtr;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("fbtree-sys/include/wrap.h");

        type FbU64;

        fn fbtree_u64_new() -> UniquePtr<FbU64>;
        unsafe fn fbtree_u64_upsert(tree: *mut FbU64, key: u64, value: u64);
        unsafe fn fbtree_u64_update(tree: *mut FbU64, key: u64, value: u64);
        unsafe fn fbtree_u64_lookup(tree: *mut FbU64, key: u64, value: *mut u64) -> bool;

        type FbString;

        fn fbtree_string_new() -> UniquePtr<FbString>;
        unsafe fn fbtree_string_upsert(
            tree: *mut FbString,
            key: *mut c_char,
            keylen: usize,
            value: u64,
        );
        unsafe fn fbtree_string_update(
            tree: *mut FbString,
            key: *mut c_char,
            keylen: usize,
            value: u64,
        );
        unsafe fn fbtree_string_lookup(
            tree: *mut FbString,
            key: *mut c_char,
            keylen: usize,
            value: *mut u64,
        ) -> bool;
    }
}

pub struct FbU64(UniquePtr<ffi::FbU64>);

unsafe impl Send for FbU64 {}
unsafe impl Sync for FbU64 {}

impl Default for FbU64 {
    fn default() -> Self {
        Self(ffi::fbtree_u64_new())
    }
}

impl FbU64 {
    #[inline]
    pub fn upsert(&self, key: u64, value: u64) {
        unsafe {
            ffi::fbtree_u64_upsert(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn update(&self, key: u64, value: u64) {
        unsafe {
            ffi::fbtree_u64_update(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn lookup(&self, key: u64) -> Option<u64> {
        unsafe {
            let mut value = 0u64;
            ffi::fbtree_u64_lookup(self.0.as_mut_ptr(), key, &mut value).then_some(value)
        }
    }
}

pub struct FbString(UniquePtr<ffi::FbString>);

unsafe impl Send for FbString {}
unsafe impl Sync for FbString {}

impl Default for FbString {
    fn default() -> Self {
        Self(ffi::fbtree_string_new())
    }
}

impl FbString {
    #[inline]
    pub fn upsert(&self, key: &str, value: u64) {
        unsafe {
            ffi::fbtree_string_upsert(
                self.0.as_mut_ptr(),
                key.as_ptr().cast_mut().cast(),
                key.len(),
                value,
            );
        }
    }

    #[inline]
    pub fn update(&self, key: &str, value: u64) {
        unsafe {
            ffi::fbtree_string_update(
                self.0.as_mut_ptr(),
                key.as_ptr().cast_mut().cast(),
                key.len(),
                value,
            );
        }
    }

    #[inline]
    pub fn lookup(&self, key: &str) -> Option<u64> {
        unsafe {
            let mut value = 0u64;
            ffi::fbtree_string_lookup(
                self.0.as_mut_ptr(),
                key.as_ptr().cast_mut().cast(),
                key.len(),
                &mut value,
            )
            .then_some(value)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::FbString;
    use crate::FbU64;

    #[test]
    fn smoke_u64() {
        let map = FbU64::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            map.upsert(i, i);
        }

        for i in 0..COUNT {
            assert_eq!(map.lookup(i), Some(i));
        }
    }

    #[test]
    fn smoke_string() {
        let map = FbString::default();

        const COUNT: u64 = 100_000;

        for i in 0..COUNT {
            map.upsert(&format!("key{i}"), i);
        }

        for i in 0..COUNT {
            assert_eq!(map.lookup(&format!("key{i}")), Some(i));
        }
    }
}
