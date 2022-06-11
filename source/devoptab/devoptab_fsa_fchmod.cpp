#include "devoptab_fs.h"

int __fsa_fchmod(struct _reent *r,
                 void *fd,
                 mode_t mode) {
    // TODO: FSChangeMode and FSStatFile?
    r->_errno = ENOSYS;
    return -1;
}
