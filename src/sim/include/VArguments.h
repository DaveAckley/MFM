/*                                              -*- mode:C++ -*-
  VArguments.h Command Line Argument registry
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file VArguments.h Command Line Argument registry
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef VARGUMENTS_H
#define VARGUMENTS_H

#include "itype.h"

namespace MFM
{
#define VARGUMENTS_MAX_SIZE 64

  /**
   * A typedef describing a function callback used to handle the
   * argument of a command line switch.
   */
  typedef void (*VArgumentHandleValue)(const char*, void* arg);

  /**
   * A class which holds a variable number of command line
   * arguments. This is normally used when a driver needs to have
   * unique command line arguments.
   */
  class VArguments
  {
  private:

    /**
     * A struct representing a single registry of a command line
     * argument, meant to be used internally by VArguments.
     */
    struct VArg
    {

      /**
       * The descriptions of each installed command line arguments.
       */
      const char* m_description;

      /**
       * The flags of each command line argument, separated by the '|'
       * character. For instance, "-h|--help" would match both "-h"
       * and "--help".  If m_filter is NULL, this entry is a 'section
       * header', used only when providing help, rather than an actual
       * argument definition.
       */
      const char* m_filter;

      /**
       * The output values gotten from the command line.
       */
      const char* m_value;

      /**
       * Flags which describe whether or not a particular argument needs
       * a following argument or not.
       */
      bool m_argsNeeded;

      /**
       * If this argument appears during processing, will be true.
       */
      bool m_appeared;

      /**
       * This will be given to the specified function upon calling.
       */
      void* m_handlerArg;

      /**
       * The functioncs called when a command line flag is encountered.
       */
      VArgumentHandleValue m_function;

      VArg() : m_description(0), m_filter(0), m_value(0),
	       m_argsNeeded(false), m_appeared(false),
	       m_handlerArg(0), m_function(0)
      {

      }



    };

    /**
     * The VArg arguments which are currently registered.
     */
    VArg m_argDescriptors[VARGUMENTS_MAX_SIZE];

    /**
     * The number of arguments and sections that have been registered
     * in this instance of VArguments.
     */
    u32 m_heldArguments;

  public:

    /**
     * Constructs a new empty VArguments collection.
     */
    VArguments();

    /**
     * Fails, to be called upon an illegal command line argument. This
     * method terminates wiht exit(0) .
     *
     * @param format The format used to print a printf-style format
     *               string describing the failure.
     */
    void Die(const char * format, ...);

    /**
     * Registers a new command line argument to this VArguments instance.
     *
     * @param description The description of this argument, printed out
     *                    upon displaying this program's usage.
     *
     * @param filter The filter used to differentiate between command
     *               line arguments, separated by the '|' character. For
     *               instance, "-h|--help" matches both "-h" and
     *               "--help".
     *
     * @param func The function called upon encounterin this command
     *             line argument when given by the user. This is called
     *             on the value gotten by the user, unless otherwise
     *             specified.
     *
     * @param handlerArg The argument specified to func upon being
     *                   called.
     *
     * @param runFunc If true, will ask for one command line
     *                argument. If not, will treat as a flag and simply
     *                execute func with NULL as its argument.
     */
    void RegisterArgument(const char* description, const char* filter,
			  VArgumentHandleValue func, void* handlerArg,
			  bool runFunc);

    /**
     * Defines a new command line argument section in this VArguments
     * instance.  Subsequent RegisterArgument calls will be described
     * as part of section \a label, unless and until another
     * RegisterSection call.
     *
     * @param label The name of the section, printed out as part of
     *                    program usage help.
     */
    void RegisterSection(const char* label);

    /**
     * To be called after all arguments are registered. This will parse
     * all command line arguments given into the internally held format.
     *
     * @param argc The amount of command line arguments in argv.
     *
     * @param argv The command line arguments.
     */
    void ProcessArguments(u32 argc, const char** argv);

    /**
     * Retrieve a command line argument which has been generated by
     * Gather. If this argument is not found, FAIL(ILLEGAL_ARGUMENT) is
     * invoked.
     *
     * @param argName The name of the argument to retrieve from this
     *                VArguments.
     *
     * @returns The value at the specified argument, or NULL if this
     *          argument was not specified by the user.
     */
    const char* Get(const char* argName) const;

    /**
     * Retrieve a command line argument which has been generated by
     * Gather. If this argument is not found, FAIL(ILLEGAL_ARGUMENT)
     * is invoked. The argument retrieved is passed through atoi and
     * the value is returned.
     *
     * @param argName The name of the argument to retrieve from this
     *                VArguments.
     *
     * @returns The value at the specified argument after passed
     *          through atoi, or NULL if this argument was not
     *          specified by the user.
     */
    u32 GetInt(const char* argName) const;

    /**
     * Checks to see if a particular argument was encountered on the
     * command line during ProcessArguments .
     *
     * @param argName The name of the argument being checked for
     *                command line membership.
     *
     * @returns \c true if this argument was encountered on the
     *          command line, else \c false.
     */
    bool Appeared(const char* argName) const;

    /**
     * Prints all held arguments to stdout. This is called when the
     * default 'help' command line argument is invoked.
     */
    void Usage() const;
  };
}

#endif /* VARGUMENTS_H */
