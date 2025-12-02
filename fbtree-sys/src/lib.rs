use cxx::UniquePtr;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("fbtree-sys/include/wrap.h");

        type FbU64;
        type FbU64Iter;

        fn fbtree_u64_new() -> UniquePtr<FbU64>;
        unsafe fn fbtree_u64_upsert(tree: *mut FbU64, key: u64, value: u64);
        unsafe fn fbtree_u64_update(tree: *mut FbU64, key: u64, value: u64);
        unsafe fn fbtree_u64_lookup(tree: *mut FbU64, key: u64, value: *mut u64) -> bool;

        unsafe fn fbtree_u64_iter(tree: *mut FbU64, key: u64) -> UniquePtr<FbU64Iter>;
        unsafe fn fbtree_u64_iter_end(iter: *mut FbU64Iter) -> bool;
        unsafe fn fbtree_u64_iter_advance(iter: *mut FbU64Iter);
        unsafe fn fbtree_u64_iter_get(iter: *mut FbU64Iter) -> u64;

        type FbString;
        type FbStringIter;

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
        unsafe fn fbtree_string_iter(
            tree: *mut FbString,
            key: *mut c_char,
            keylen: usize,
        ) -> UniquePtr<FbStringIter>;
        unsafe fn fbtree_string_iter_end(iter: *mut FbStringIter) -> bool;
        unsafe fn fbtree_string_iter_advance(iter: *mut FbStringIter);
        unsafe fn fbtree_string_iter_get(iter: *mut FbStringIter) -> u64;
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

    #[inline]
    pub fn iter(&self, key: u64) -> FbU64Iter {
        FbU64Iter(unsafe { ffi::fbtree_u64_iter(self.0.as_mut_ptr(), key) })
    }
}

pub struct FbU64Iter(UniquePtr<ffi::FbU64Iter>);

impl Iterator for FbU64Iter {
    type Item = u64;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        if unsafe { ffi::fbtree_u64_iter_end(self.0.as_mut_ptr()) } {
            return None;
        }

        let value = unsafe { ffi::fbtree_u64_iter_get(self.0.as_mut_ptr()) };
        unsafe { ffi::fbtree_u64_iter_advance(self.0.as_mut_ptr()) };
        Some(value)
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

    #[inline]
    pub fn iter(&self, key: &str) -> FbStringIter {
        FbStringIter(unsafe {
            ffi::fbtree_string_iter(
                self.0.as_mut_ptr(),
                key.as_ptr().cast_mut().cast(),
                key.len(),
            )
        })
    }
}

pub struct FbStringIter(UniquePtr<ffi::FbStringIter>);

impl Iterator for FbStringIter {
    type Item = u64;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        if unsafe { ffi::fbtree_string_iter_end(self.0.as_mut_ptr()) } {
            return None;
        }

        let value = unsafe { ffi::fbtree_string_iter_get(self.0.as_mut_ptr()) };
        unsafe { ffi::fbtree_string_iter_advance(self.0.as_mut_ptr()) };
        Some(value)
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

        assert!(map.iter(0).eq(0..COUNT));
    }

    #[test]
    fn smoke_string() {
        let map = FbString::default();

        const COUNT: u64 = 100_000;

        let keys = (0..COUNT).map(|i| format!("key{i:06}")).collect::<Vec<_>>();

        for (i, key) in keys.iter().enumerate() {
            map.upsert(key, i as u64);
        }

        for (i, key) in keys.iter().enumerate() {
            assert_eq!(map.lookup(key), Some(i as u64));
        }

        assert!(map.iter("key000005").eq(5..COUNT));
    }
}
