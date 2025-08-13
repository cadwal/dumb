;;; dumb-autoins.el --- Auto-insertion for DUMB .c and .h files

;; Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>

;; Author: Kalle O. Niemitalo <tosi@stekt.oulu.fi>
;; Keywords: local, c

;; This file is part of DUMB, a Doom-like 3D game engine.

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to
;; the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;; Commentary:

;; 

;;; Code:

(defun dumb-header-name-to-id (filename)
  "Translate FILENAME to a C identifier for use in an include guard.
Removes \"include/\" from the beginning, converts the filename to
upper case and replaces [^A-Z_0-9] with underscores.  Prepends \"_\"
if the resulting identifier would otherwise begin with a digit."
  (let ((id (upcase filename)))
    (if (string-match "^INCLUDE/" id)
	(setq id (replace-match "" t t id)))
    (while (string-match "[^A-Z_0-9]" id)
      (setq id (replace-match "_" t t id)))
    (if (string-match "^[0-9]" id)
	(setq id (concat "_" id)))
    id))

(define-auto-insert '("/dumb-[^/]+/\\(.+\\.c\\'\\)" . "DUMB C source")
  '("Short description: "
    '(setq v1 (match-string 1 buffer-file-name))
    "\
/* DUMB: A Doom-like 3D game engine.
 *
 * " v1 ": " str "
 * Copyright (C) " (format-time-string "%Y") " by " user-full-name " <" user-mail-address ">
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include \"libdumbutil/dumb-nls.h\"

" _ "/* put the following in main() if you have that */
#ifdef ENABLE_NLS
   setlocale(LC_ALL, \"\");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */

// Local Variables:
// c-basic-offset: 3
// End:
" '(setq c-basic-offset 3)))

(define-auto-insert '("/dumb-[^/]+/\\(.+\\.h\\'\\)" . "DUMB C header")
  '("Short description: "
    '(setq v1 (match-string 1 buffer-file-name))
    "\
/* DUMB: A Doom-like 3D game engine.
 *
 * " v1 ": " str "
 * Copyright (C) " (format-time-string "%Y") " by " user-full-name " <" user-mail-address ">
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#ifndef " (dumb-header-name-to-id v1) "
#define " (dumb-header-name-to-id v1) "

" _ "

#endif /* " (dumb-header-name-to-id v1) " */

// Local Variables:
// c-basic-offset: 3
// End:
" '(setq c-basic-offset 3)))

;;; This formfeed stops Emacs from reading the Local Variables section above.


;;; dumb-autoins.el ends here