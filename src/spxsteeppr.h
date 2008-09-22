/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1997-1999 Roland Wunderling                              */
/*                  1997-2008 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: spxsteeppr.h,v 1.21 2008/09/22 20:43:19 bzfpfets Exp $"


/**@file  spxsteeppr.h
 * @brief Steepest edge pricer.
 */
#ifndef _SPXSTEEPPR_H_
#define _SPXSTEEPPR_H_


#include <assert.h>

#include "spxdefines.h"
#include "spxpricer.h"
#include "random.h"

namespace soplex
{

/**@brief   Steepest edge pricer.
   @ingroup Algo
      
   Class SPxSteepPR implements a steepest edge pricer to be used with
   SoPlex.
   
   See SPxPricer for a class documentation.
*/
class SPxSteepPR : public SPxPricer
{
public:

   //-------------------------------------
   /**@name Types */
   //@{
   /// How to setup the direction multipliers.
   /** Possible settings are #EXACT for starting with exactly computed
       values, or #DEFAULT for starting with multipliers set to 1. The
       latter is the default.
   */
   enum Setup {
      EXACT,   ///< starting with exactly computed values
      DEFAULT  ///< starting with multipliers set to 1
   };
   //@}

private:

   //-------------------------------------
   /**@name Data */
   //@{
   /// vector of pricing penalties
   DVector penalty;
   /// vector of pricing penalties
   DVector coPenalty;
   /// working vector
   DVector workVec;
   /// working vector
   SSVector workRhs;
   ///
   Real pi_p;
   ///
   int prefSetup;
   /// preference multiplier for selecting as pivot
   DataArray < Real > coPref;
   /// preference multiplier for selecting as pivot
   DataArray < Real > pref;
   ///
   DataArray < Real > leavePref;
   /// setup type.
   Setup setup;
   /// accuracy for computing steepest directions.
   Real accuracy;
   //@}

   //-------------------------------------
   /**@name Preferences */
   //@{
   ///
   void setupPrefsX(Real mult, Real /*tie*/, Real /*cotie*/, Real shift, Real coshift);
   ///
   void setupPrefs(SPxSolver::Type);
   //@}

public:

   //-------------------------------------
   /**@name Construction / destruction */
   //@{
   ///
   SPxSteepPR()
      : SPxPricer("Steep")
      , workRhs (0, 1e-16)
      , setup (DEFAULT)
      , accuracy(1e-4)
   {}
   /// destructor
   virtual ~SPxSteepPR()
   {}
   //@}

   //-------------------------------------
   /**@name Access / modification */
   //@{
   /// sets the solver
   virtual void load(SPxSolver* base);
   /// clear solver and preferences
   virtual void clear();
   /// set entering/leaving algorithm
   virtual void setType(SPxSolver::Type);
   /// set row/column representation
   virtual void setRep(SPxSolver::Representation rep);
   ///
   virtual int selectLeave();
   ///
   virtual void left4(int n, SPxId id);
   ///
   virtual SPxId selectEnter();
   ///
   virtual void entered4(SPxId id, int n);
   /// \p n vectors have been added to loaded LP.
   virtual void addedVecs (int n);
   /// \p n covectors have been added to loaded LP.
   virtual void addedCoVecs(int n);
   /// \p the i'th vector has been removed from the loaded LP.
   virtual void removedVec(int i);
   /// \p the i'th covector has been removed from the loaded LP.
   virtual void removedCoVec(int i);
   /// \p n vectors have been removed from loaded LP.
   virtual void removedVecs(const int perm[]);
   /// \p n covectors have been removed from loaded LP.
   virtual void removedCoVecs(const int perm[]);
   //@}

#ifndef NO_CONSISTENCY_CHECKS
   //-------------------------------------
   /**@name Consistency check */
   //@{
   ///
   virtual bool isConsistent() const;
   //@}
#endif

private:

   //-------------------------------------
   /**@name Blocked */
   //@{
   /// copy constructor
   SPxSteepPR( const SPxSteepPR& );
   /// assignment operator
   SPxSteepPR& operator=( const SPxSteepPR& );
   //@}

};

} // namespace soplex
#endif // _SPXSTEEPPR_H_

//-----------------------------------------------------------------------------
//Emacs Local Variables:
//Emacs mode:c++
//Emacs c-basic-offset:3
//Emacs tab-width:8
//Emacs indent-tabs-mode:nil
//Emacs End:
//-----------------------------------------------------------------------------
