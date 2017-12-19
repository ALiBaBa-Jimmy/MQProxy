             --------------------------------------------------
                   --- Manual Additions/Modifications ---
             --------------------------------------------------
                       PC-lint for C/C++ Version 8.00h

    This readme.txt supplements the on-line PC-lint manual entitled
    "Reference Manual for PC-lint/Flexelint" found in the installation
    directory under the name "pc-lint.pdf"


                  ------ Support for Microsoft .NET ------

    As of patch level 8.00f we support Microsoft .NET.  There is a new
    compiler options file (co-msc70.lnt) for Visual C 7.0 and a new
    environment file (env-vc7.lnt) for Visual Studio 7.0.  A few new
    keywords have been added (__if_exists and __if_not_exists) and we
    can translate Visual C++ project files (with a .vcproj extension) to
    .lnt project files.  That is, when a file with a .vcproj extension
    is given to PC-lint, the .lnt project file will appear on standard
    out.  This is done in a manner similar to .dsp files (see section
    3.6 of the manual).  See also env-vc7.lnt to see how a tool can be
    created to automate the process.

    Patch level 8.00g fixed a few glitches.  The __if_exists 'statement'
    is not really a statement since it appears in a number of different
    contexts and so that had to be rewritten at the token level.  The
    .vcproj files had a little wrinkle.  We previously looked for
    "Source Files" but this categorization could not be relied on so now
    we just look for files with any of our C++ extents (extents can be
    added by the user via the +cpp option) or a .c extent.


                          ------ What's New ------

    To find out what we've added to the product since Version 7.50,
    check out Chapter 18 "What's New" in the Reference Manual.


                           ------ Front End ------

    Your linting experience will be considerably enhanced by adapting
    your favorite editor or compiler environment to the task of
    sequencing from error to error.  See Section 3.5 of the Reference
    Manual.


                        ------ Multiple Passes ------

    By default, PC-lint/FlexeLint will go through all your modules in
    one pass.  For projects not previously linted there will be enough
    messages to look at.  However, with just one pass, you will not be
    taking full advantage of our new interfunction value tracking.  With
    just one pass, we will not know about dangerous return values for
    functions that are defined later than they are called, and we will
    not know about dangerous arguments for functions that are defined
    early.  To introduce a second pass you need only to add the command
    line option:

        -passes(2)

    or, if this syntax presents a problem with your Shell, you may use:

        -passes[2]

    or, in some cases,

        -passes=2

    is needed.

    See Section 9.2.2, "Interfunction Value Tracking".



                      ------ Additional Options ------

    The following options have been added.

    o  Explicit Throw flag (+fet)
       In early releases of 8.0 (8.0d and earlier) we provided a method
       of checking exception specifications to prevent exception leaks,
       etc.  Unfortunately we were too aggressive and a number of
       programmers pointed out that since a function that doesn't
       declare to throw can throw any exception, then it is at least
       theoretically possible that by adding an exception specification
       to a function that doesn't have one, you will induce the dreaded
       unexpected() call.

       So we made the appropriate modifications and we began getting the
       opposite complaint.  Some programmers were quite happy with the
       old system since it allowed them to track and control their own
       exceptions especially in situations where library functions with
       undocumented exceptions were non-existent.

       To resolve the problem and make everyone happy, version 8.00g has
       a new flag.  The Explicit Throw flag (+fet) is normally OFF.  If
       the flag is OFF then the absence of an exception specification
       (the throw list for a function) is treated as a declaration that
       the function can throw any exception.  This is standard C++.  If
       the flag is ON, however, the function is assumed to throw no
       exception.  In effect, the flag says that any exception thrown
       must be explicitly declared.  Consider
       
             double sqrt( double x ) throw( overflow );
             double abs( double x );
             double f( double x )
                 {
                 return sqrt( abs(x) );
                 }
       
       In this example, sqrt() has an exception specification that
       indicates that it throws only one exception (overflow) and no
       others.  The functions abs() and f(), on the other hand, have no
       exception specification, and are, therefore, assumed to
       potentially throw all exceptions.  With the Explicit Throw flag
       OFF you will receive no warning.  With the flag ON (with a +fet),
       you will receive Warning 1550 that exception overflow is not on
       the throw list of function f().

       The advantage of turning this flag ON is that the programmer can
       obtain better control of his exception specifications and can
       keep them from propogating too far up the call stack.  This style
       of analysis is very similar to that employed quite successfully
       by Java.

       The disadvantage, however, is that by adding an exception
       specification you are saying that the function throws no
       exception other than those listed.  If a library function throws
       an undeclared exception (such as abs() above) you will get the
       dreaded unexpected() function call.  See Scott Meyers "More
       Effective C++", Item 14.

       Can you have the best of both worlds?  Through the magic of
       macros it would appear that you can.  Define
       
             #define Throws(X) throw X
       
       which then is used as:
       
             float f( float x ) Throws((overflow,underflow))
                 { ...
       
       Notice the required double set of parentheses which are needed to
       get an arbitrary list of exceptions into a single macro.

       When you compile you can define Throws to be null and when you
       lint you can define Throws as above.  This can be done most
       easily by doing a #ifdef on the _lint preprocessor variable
       (defined while running our product).


                  ------ New or Improved Error Messages ------


       686  Option 'String' is suspicious because of 'Name' -- An option
            is considered suspicious for one of a variety of reasons.
            The reason is designated by a reason code that is specified
            by Name.  At this writing, the only reason code is
            'unbalanced quotes'.
       1076 Anonymous union assumed to be 'static' -- Anonymous unions
            need to be declared static.  This is because the names
            contained within are considered local to the module in which
            they are declared.

       1549 Exception thrown for function 'Symbol' not declared to throw
            -- An exception was thrown (i.e., a throw was detected)
            within a function and not within a try block;  moreover the
            function was declared to throw but the exception thrown was
            not on the list.  If you provide an exception specification,
            include all the exception types you potentially will throw.
            [23, Item 14]

       1550 exception 'Name' thrown by function 'Symbol' is not on
            throw-list of function 'Symbol' -- A function was called
            (first Symbol) which was declared as potentially throwing an
            exception.  The call was not made from within a try block
            and the function making the call had an exception
            specification.  Either add the exception to the list, or
            place the call inside a try block and catch the throw.  [23,
            Item 14]

       1560 Uncaught exception 'Name' not on throw-list of function
            'Symbol' -- A direct or indirect throw of the named
            exception occurred within a try block and was either not
            caught by any handler or was rethrown by the handler.
            Moreover, the function has an exception specification and
            the uncaught exception is not on the list.  Note that a
            function that fails to declare a list of thrown exceptions
            is assumed to potentially throw any exception.




                           Gimpel Software
                           June, 2002
