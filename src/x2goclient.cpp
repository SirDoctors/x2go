/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*   Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de>                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>

#include "unixhelper.h"
#include "ongetpass.h"

int wrap_x2go_main (int argc, char **argv) {
  return (x2goMain (argc, argv));
}

#ifdef Q_OS_UNIX
int fork_helper (int argc, char **argv) {
  /* Fork off to start helper process. */
  pid_t tmp_pid = fork ();

  /* Child. */
  if (0 == tmp_pid) {
    /* Starting unixhelper. */
    std::vector<std::string> new_argv;
    new_argv.push (std::string (argv[0]));
    new_argv.push ("--unixhelper");

    std::vector<const char *> new_argv_c_str;
    for (const_iterator it = new_argv.begin (); it != new_argv.end (); ++it) {
      new_argv_c_str.push ((*it).c_str ());
    }
    new_argv_c_str.push ("");

    if (0 != execv (new_argv_c_str.data (), new_argv_c_str.data ())) {
      std::cerr << "Failed to re-execute process as UNIX cleanup helper tool: " << std::strerror (errno) << "\n"
                << "Terminating and killing parent." << "\n"
                << "Please report a bug, refer to this documentation: http://wiki.x2go.org/doku.php/wiki:bugs" << std::endl;

      pid_t parent_pid = getppid ();
      if (0 != kill (parent_pid, SIGTERM)) {
        std::cerr << "Failed to kill parent process: " << std::strerror (errno) << std::endl;
      }

      std::exit (EXIT_FAILURE);
    }

    /* Anything here shall be unreachable. */
  }
  /* Error. */
  else if (-1 == tmp_pid) {
    std::cerr << "Unable to create a new process for the UNIX cleanup watchdog: " << std::strerror (errno) << "\n";
    std::cerr << "Terminating. Please report a bug, refer to this documentation: http://wiki.x2go.org/doku.php/wiki:bugs" << std::endl;

    std::exit (EXIT_FAILURE);
  }
  /* Parent. */
  else {
    /* Start real X2Go Client. */
    return (wrap_x2go_main (argc, argv));
  }
}
#endif /* defined (Q_OS_UNIX) */

int main (int argc, char **argv) {
#ifdef Q_OS_UNIX
  /* Scan program arguments for --unixhelper flag. */
  bool unix_helper_request = 0;
  for (int i = 0; i < argc; ++i) {
    std::string cur_arg (argv[i]);

    /* Make the current argument lowercase. */
    std::transform (cur_arg.begin (), cur_arg.end (), cur_arg.begin (), std::tolower);

    if ((!cur_arg.empty ()) && (cur_arg.compare ("--unixhelper"))) {
      unix_helper_request = 1;
      break;
    }
  }

  if (unix_helper_request) {
    /* We were instructed to start as the UNIX cleanup helper tool. */
    return (unixhelper::unix_cleanup ());
  }
  else {
    /*
     * setsid() may succeed and we become a session and process
     * group leader, or it may fail indicating that we already
     * are a process group leader. Either way is fine.
     */
    setsid ();

    /* We should be process group leader by now. */
    return (fork_helper (argc, argv));
  }
#else /* defined (Q_OS_UNIX) */
  return (wrap_x2go_main (argc, argv));
#endif /* defined (Q_OS_UNIX) */
}
