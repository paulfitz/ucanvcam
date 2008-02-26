
#include <string>

extern std::string getRegistry(const char *key, bool *found = NULL);

extern bool putRegistry(const char *key, const char *value);

extern bool unputRegistry(const char *key);



