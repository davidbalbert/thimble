struct SleepLock {
    uint locked;
    SpinLock lock;
}
