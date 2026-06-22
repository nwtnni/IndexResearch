use cxx::UniquePtr;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("parlayhash-sys/include/wrap.h");

        type ParlayUnorderedMap;

        fn unorderedmap_new() -> UniquePtr<ParlayUnorderedMap>;

        unsafe fn map_upsert(map: *mut ParlayUnorderedMap, key: u64, value: u64);
        unsafe fn map_update(map: *mut ParlayUnorderedMap, key: u64, value: u64);
        unsafe fn map_find(map: *mut ParlayUnorderedMap, key: u64) -> Result<u64>;
    }
}

pub struct ParlayMap(UniquePtr<ffi::ParlayUnorderedMap>);

unsafe impl Send for ParlayMap {}
unsafe impl Sync for ParlayMap {}

impl Default for ParlayMap {
    fn default() -> Self {
        Self(ffi::unorderedmap_new())
    }
}

impl ParlayMap {
    #[inline]
    pub fn upsert(&self, key: u64, value: u64) {
        unsafe {
            ffi::map_upsert(self.0.as_mut_ptr(), key, value);
        }
    }

    #[inline]
    pub fn update(&self, key: u64, value: u64) {
        unsafe {
            ffi::map_update(self.0.as_mut_ptr(), key, value);
        }

