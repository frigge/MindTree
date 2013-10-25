def MTsignal(id_str):
    '''simple trick to allow decorator style signal registration'''
    def decorator(fn):
        MT.attachToSignal(id_str, fn).detach()
        return fn
    return decorator

import MT
MT.__dict__["signal"] = MTsignal
