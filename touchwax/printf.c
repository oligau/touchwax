/* http://stackoverflow.com/questions/8924831/iphone-debugging-real-device */
int printf(const char * __restrict format, ...)
{ 
    va_list args;
    va_start(args,format);    
    NSLogv([NSString stringWithUTF8String:format], args) ;    
    va_end(args);
    return 1;
}
