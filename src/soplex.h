/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1997-1999 Roland Wunderling                              */
/*                  1997-2002 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: soplex.h,v 1.67 2005/11/01 21:27:04 bzforlow Exp $"

/**@file  soplex.h
 * @brief preconfigured \ref soplex::SoPlex "SoPlex" LP-solver.
 */
#ifndef _SOPLEX_H_
#define _SOPLEX_H_

#include <assert.h>

#include "spxsolver.h"
#include "spxscaler.h"
#include "spxsimplifier.h"
#include "spxsteeppr.h"
#include "spxfastrt.h"
#include "spxweightst.h"
#include "slufactor.h"

namespace soplex
{

/**@class SoPlex
   @brief   Preconfigured #SoPlex LP-solver.
   @ingroup Algo
*/
class SoPlex : public SPxLP
{
protected:

   //-------------------------
   //**@name Protected data */
   //@{
   SPxFastRT       m_fastRT;     ///< fast ratio test
   SPxSteepPR      m_steepPR;    ///< steepest edge pricing
   //   SPxWeightST st;  ///< weight starter
   SLUFactor       m_slu;        ///< LU Factorisation
   SPxSolver       m_solver;     ///< solver
   SPxScaler*      m_preScaler;  ///< pre-scaler
   SPxScaler*      m_postScaler; ///< post-scaler
   SPxSimplifier*  m_simplifier; ///< simplifier
   bool            m_vanished;   ///< did the presolver solve the problem ?
   //@}

public:

   //---------------------------------------
   //**@name Construction / destruction */
   //@{
   /// default construtor.
   explicit SoPlex(
      SPxSolver::Type           type = SPxSolver::LEAVE, 
      SPxSolver::Representation rep  = SPxSolver::COLUMN );
   virtual ~SoPlex();
   //@}

   //---------------------------------------
   //**@name Access / modification */
   //@{
   /// set update type for factorization.
   virtual void setUtype(SLUFactor::UpdateType tp)
   {
      m_slu.setUtype(tp);
   }
   /// return current Pricing.
   inline SPxSolver::Pricing pricing() const
   {
      return m_solver.pricing();
   }
   /// set FULL or PARTIAL pricing.
   virtual void setPricing(SPxSolver::Pricing pr)
   {
      m_solver.setPricing(pr);
   }
   /// return current Type.
   inline SPxSolver::Type type() const
   {
      return m_solver.type();
   }
   /// set LEAVE or ENTER algorithm.
   virtual void setType(SPxSolver::Type tp)
   {
      m_solver.setType(tp);
   }
   /// setup prescaler to use.
   virtual void setPreScaler(SPxScaler* scaler);
   /// setup postscaler to use.
   virtual void setPostScaler(SPxScaler* scaler);
   /// setup simplifier to use.
   virtual void setSimplifier(SPxSimplifier* simpli);
   /// has a simplifier been set?
   inline bool has_simplifier() const
   {
      return m_simplifier != 0;
   }
   /// has a prescaler been set?
   inline bool has_prescaler() const
   {
      return m_preScaler != 0;
   }
   /// has a postscaler been set?
   inline bool has_postscaler() const
   {
      return m_postScaler != 0;
   }
   /// setup pricer to use.
   virtual void setPricer(SPxPricer* pricer)
   {
      m_solver.setPricer(pricer);
   }
   /// setup ratio-tester to use.
   virtual void setTester(SPxRatioTester* tester)
   {
      m_solver.setTester(tester);
   }
   /// setup starting basis generator to use.
   virtual void setStarter(SPxStarter* starter)
   {
      m_solver.setStarter(starter);
   }
   /// set time limit.
   virtual void setTerminationTime(Real time = infinity)
   {
      m_solver.setTerminationTime(time);
   }
   /// return time limit.
   inline Real terminationTime() const
   {
      return m_solver.terminationTime();
   }
   /// set iteration limit.
   virtual void setTerminationIter(int iter = -1)
   {
      m_solver.setTerminationIter(iter);
   }
   /// return iteration limit.
   inline int terminationIter() const
   {
      return m_solver.terminationIter();
   }
   /// set objective limit.
   virtual void setTerminationValue(Real val = infinity)
   {
      m_solver.setTerminationValue(val);
   }
   /// return objective limit.
   inline Real terminationValue() const
   {
      return m_solver.terminationValue();
   }
   /// allowed bound violation for optimal solution.
   /** When no vector violates its bound by more than \f$\delta\f$,
    *  the basis is considered optimal.
    */
   virtual Real delta() const
   {
      return m_solver.delta();
   }
   /// set parameter \p delta.
   virtual void setDelta(Real d)
   {
      m_solver.setDelta(d);
   }
   //@}

   //---------------------------------------
   //**@name Solving and solution query */
   //@{
   ///
   virtual SPxSolver::Status solve();
   ///
   virtual Real objValue() const;
   ///
   virtual SPxSolver::Status getPrimal(Vector& vector) const;
   ///
   virtual SPxSolver::Status getSlacks(Vector& vector) const;
   ///
   virtual SPxSolver::Status getDual(Vector& vector) const;
   ///
   virtual SPxSolver::Status getRedCost(Vector& vector) const;

   ///
   virtual SPxSolver::Status getDualfarkas(Vector& vector) const;

   /// get violation of constraints.
   virtual void qualConstraintViolation(Real& maxviol, Real& sumviol) const;
   /// get violations of bounds.
   virtual void qualBoundViolation(Real& maxviol, Real& sumviol) const;
#if 0
   /// get the residuum |Ax-b|.
   virtual void qualSlackViolation(Real& maxviol, Real& sumviol) const;
   /// get violation of optimality criterion.
   virtual void qualRedCostViolation(Real& maxviol, Real& sumviol) const;
#endif
   /// time spent in factorizations
   virtual Real getFactorTime() const
   {
      return m_vanished ? REAL(0.0) : m_slu.getFactorTime();
   }
   /// number of factorizations performed
   virtual int getFactorCount() const
   {
      return m_vanished ? 0 : m_slu.getFactorCount();
   }
   /// time spent in factorizations
   virtual Real getSolveTime() const
   {
      return m_vanished ? REAL(0.0) : m_slu.getSolveTime();
   }
   /// number of factorizations performed
   virtual int getSolveCount() const
   {
      return m_vanished ? 0 : m_slu.getSolveCount();
   }
   ///
   virtual int iteration() const
   {      
      return m_vanished ? 0 : m_solver.basis().iteration();
   }
   ///
   virtual bool terminate() 
   {
      return m_solver.terminate();
   }
   /// returns the current status
   virtual SPxSolver::Status status() const
   {
      if (m_vanished)
         return SPxSolver::OPTIMAL;

      return m_solver.status();
   }
   //@}

   //---------------------------------------
   //**@name I/O */
   //@{
   /// write basis to \p filename in MPS format.
   virtual bool writeBasisFile ( const char*    filename, 
                                 const NameSet& rowNames, 
                                 const NameSet& colNames );
   //@}

private:

   //---------------------------------------
   //**@name Blocked */
   //@{
   /// assignment operator is not implemented.
   SoPlex& operator=(const SoPlex& rhs);
   /// copy constructor is not implemented.
   SoPlex(const SoPlex&);
   //@}
};
} // namespace soplex
#endif // _SOPLEX_H_

//-----------------------------------------------------------------------------
//Emacs Local Variables:
//Emacs mode:c++
//Emacs c-basic-offset:3
//Emacs tab-width:8
//Emacs indent-tabs-mode:nil
//Emacs End:
//-----------------------------------------------------------------------------
