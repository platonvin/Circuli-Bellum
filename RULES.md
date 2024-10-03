### C++ rules:
 - no logic in constructors / destructors (exception: isolated class); TODO: remove all, including default values
 - less std templates (freezes clangd lol). Write yourself (maybe from preprocessed std) //TODO modules?
 - no useless functions (return m_var)
