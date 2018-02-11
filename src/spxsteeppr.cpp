/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1996-2017 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//TODO may be faster to have a greater zero tolerance for sparse pricing vectors
//     to reduce the number of nonzero entries, e.g. for workVec

#include <assert.h>
#include <iostream>

#include "spxdefines.h"
#include "spxsteeppr.h"
#include "random.h"

#define STEEP_REFINETOL 2.0

namespace soplex
{

// #define EQ_PREF 1000

  template <class R>
void SPxSteepPR<R>::clear()
{
   this->thesolver = 0;
}

  template <class R>
  void SPxSteepPR<R>::load(SPxSolver<R>* base)
{
   this->thesolver = base;

   if (base)
   {
      workVec.clear();
      workVec.reDim(base->dim());
      workRhs.clear();
      workRhs.reDim(base->dim());
   }
}

  template <class R>
void SPxSteepPR<R>::setType(typename SPxSolver<R>::Type type)
{
   workRhs.setEpsilon(this->thesolver->epsilon());

   setupWeights(type);
   workVec.clear();
   workRhs.clear();
   refined = false;

   bestPrices.clear();
   bestPrices.setMax(this->thesolver->dim());
   prices.reMax(this->thesolver->dim());

   if( type == SPxSolver<R>::ENTER )
   {
      bestPricesCo.clear();
      bestPricesCo.setMax(this->thesolver->coDim());
      pricesCo.reMax(this->thesolver->coDim());
   }
}

  template <class R>
void SPxSteepPR<R>::setupWeights(typename SPxSolver<R>::Type type)
{
   int i;
   int endDim = 0;
   int endCoDim = 0;
   DVector& weights = this->thesolver->weights;
   DVector& coWeights = this->thesolver->coWeights;

   if( setup == DEFAULT )
   {
      if( type == SPxSolver<R>::ENTER )
      {
         if( this->thesolver->weightsAreSetup )
         {
            // check for added/removed rows and adapt norms accordingly
            if (coWeights.dim() < this->thesolver->dim())
               endDim = coWeights.dim();
            else
               endDim = this->thesolver->dim();
            if (weights.dim() < this->thesolver->coDim())
               endCoDim = weights.dim();
            else
               endCoDim = this->thesolver->coDim();
         }

         coWeights.reDim(this->thesolver->dim(), false);
         for (i = this->thesolver->dim() - 1; i >= endDim; --i)
            coWeights[i] = 2.0;
         weights.reDim(this->thesolver->coDim(), false);
         for (i = this->thesolver->coDim() - 1; i >= endCoDim; --i)
            weights[i] = 1.0;
      }
      else
      {
         assert(type == SPxSolver<R>::LEAVE);

         if( this->thesolver->weightsAreSetup )
         {
            // check for added/removed rows and adapt norms accordingly
            if (coWeights.dim() < this->thesolver->dim())
               endDim = coWeights.dim();
            else
               endDim = this->thesolver->dim();
         }

         coWeights.reDim(this->thesolver->dim(), false);
         for (i = this->thesolver->dim() - 1; i >= endDim; --i)
            coWeights[i]   = 1.0;
      }
   }
   else
   {
      MSG_INFO1( (*this->thesolver->spxout), (*this->thesolver->spxout) << " --- initializing steepest edge multipliers" << std::endl; )

      if (type == SPxSolver<R>::ENTER)
      {
         coWeights.reDim(this->thesolver->dim(), false);
         for (i = this->thesolver->dim() - 1; i >= endDim; --i)
            coWeights[i] = 1.0;
         weights.reDim(this->thesolver->coDim(), false);
         for (i = this->thesolver->coDim() - 1; i >= endCoDim; --i)
            weights[i] = 1.0 + this->thesolver->vector(i).length2();
      }
      else
      {
         assert(type == SPxSolver<R>::LEAVE);
         coWeights.reDim(this->thesolver->dim(), false);
         SSVector tmp(this->thesolver->dim(), this->thesolver->epsilon());
         for( i = this->thesolver->dim() - 1; i >= endDim && !this->thesolver->isTimeLimitReached(); --i )
         {
            this->thesolver->basis().coSolve(tmp, this->thesolver->unitVector(i));
            coWeights[i] = tmp.length2();
         }
      }
   }
   this->thesolver->weightsAreSetup = true;
}

  template <class R>
void SPxSteepPR<R>::setRep(typename SPxSolver<R>::Representation)
{
   if (workVec.dim() != this->thesolver->dim())
   {
      DVector tmp = this->thesolver->weights;
      this->thesolver->weights = this->thesolver->coWeights;
      this->thesolver->coWeights = tmp;

      workVec.clear();
      workVec.reDim(this->thesolver->dim());
   }
}

  template <class R>
void SPxSteepPR<R>::left4(int n, SPxId id)
{
   assert(this->thesolver->type() == SPxSolver<R>::LEAVE);

   if (id.isValid())
   {
      Real        delta         = 0.1 + 1.0 / this->thesolver->basis().iteration();
      Real*       coWeights_ptr = this->thesolver->coWeights.get_ptr();
      const Real* workVec_ptr   = workVec.get_const_ptr();
      const Real* rhoVec        = this->thesolver->fVec().delta().values();
      Real        rhov_1        = 1.0 / rhoVec[n];
      Real        beta_q        = this->thesolver->coPvec().delta().length2() * rhov_1 * rhov_1;

#ifndef NDEBUG
      if (spxAbs(rhoVec[n]) < theeps * 0.5)
      {
         MSG_INFO3( (*this->thesolver->spxout), (*this->thesolver->spxout) << "WSTEEP04: rhoVec = "
                           << rhoVec[n] << " with smaller absolute value than 0.5*theeps = " << 0.5*theeps << std::endl; )
      }
#endif

      const IdxSet& rhoIdx = this->thesolver->fVec().idx();
      int           len    = this->thesolver->fVec().idx().size();

      for(int i = 0; i < len; ++i)
      {
         int  j = rhoIdx.index(i);
         coWeights_ptr[j] += rhoVec[j] * (beta_q * rhoVec[j] - 2.0 * rhov_1 * workVec_ptr[j]);

         if (coWeights_ptr[j] < delta)
            coWeights_ptr[j] = delta; // coWeights_ptr[j] = delta / (1+delta-x);
         else if (coWeights_ptr[j] >= infinity)
            coWeights_ptr[j] = 1.0 / this->theeps;
      }
      coWeights_ptr[n] = beta_q;
   }
}

Real inline computePrice(Real viol, Real weight, Real tol)
{
   if( weight < tol )
      return viol * viol / tol;
   else
      return viol * viol / weight;
}

  template <class R>
int SPxSteepPR<R>::buildBestPriceVectorLeave( Real feastol )
{
   int idx;
   int nsorted;
   Real x;
   const Real* fTest = this->thesolver->fTest().get_const_ptr();
   const Real* cpen = this->thesolver->coWeights.get_const_ptr();
   typename SPxPricer<R>::IdxElement price;
   prices.clear();
   bestPrices.clear();

   // construct vector of all prices
   for (int i = this->thesolver->infeasibilities.size() - 1; i >= 0; --i)
   {
      idx = this->thesolver->infeasibilities.index(i);
      x = fTest[idx];
      if (x < -feastol)
      {
         // it might happen that we call the pricer with a tighter tolerance than what was used when computing the violations
         this->thesolver->isInfeasible[idx] = this->VIOLATED;
         price.val = computePrice(x, cpen[idx], feastol);
         price.idx = idx;
         prices.append(price);
      }
   }
   // set up structures for the quicksort implementation
   this->compare.elements = prices.get_const_ptr();
   // do a partial sort to move the best ones to the front
   // TODO this can be done more efficiently, since we only need the indices
   nsorted = SPxQuicksortPart(prices.get_ptr(), this->compare, 0, prices.size(), HYPERPRICINGSIZE);
   // copy indices of best values to bestPrices
   for( int i = 0; i < nsorted; ++i )
   {
      bestPrices.addIdx(prices[i].idx);
      this->thesolver->isInfeasible[prices[i].idx] = VIOLATED_AND_CHECKED;
   }

   if( nsorted > 0 )
      return prices[0].idx;
   else
      return -1;
}


  template <class R>
int SPxSteepPR<R>::selectLeave()
{
   assert(isConsistent());

   int retid;

   if (this->thesolver->hyperPricingLeave && this->thesolver->sparsePricingLeave)
   {
      if ( bestPrices.size() < 2 || this->thesolver->basis().lastUpdate() == 0 )
      {
         // call init method to build up price-vector and return index of largest price
         retid = buildBestPriceVectorLeave(this->theeps);
      }
      else
         retid = selectLeaveHyper(this->theeps);
   }
   else if (this->thesolver->sparsePricingLeave)
      retid = selectLeaveSparse(this->theeps);
   else
      retid = selectLeaveX(this->theeps);

   if( retid < 0 && !refined )
   {
      refined = true;
      MSG_INFO3( (*this->thesolver->spxout), (*this->thesolver->spxout) << "WSTEEP03 trying refinement step..\n"; )
      retid = selectLeaveX(this->theeps/STEEP_REFINETOL);
   }

   if( retid >= 0 )
   {
      assert( this->thesolver->coPvec().delta().isConsistent() );
      // coPvec().delta() might be not setup after the solve when it contains too many nonzeros.
      // This is intended and forcing to keep the sparsity information leads to a slowdown
      // TODO implement a dedicated solve method for unitvectors
      this->thesolver->basis().coSolve(this->thesolver->coPvec().delta(),
                                 this->thesolver->unitVector(retid));
      assert( this->thesolver->coPvec().delta().isConsistent() );
      workRhs.setup_and_assign(this->thesolver->coPvec().delta());
      this->thesolver->setup4solve(&workVec, &workRhs);
   }

   return retid;
}

  template <class R>
int SPxSteepPR<R>::selectLeaveX(Real tol)
{
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();
   const Real* fTest         = this->thesolver->fTest().get_const_ptr();
   Real best = -infinity;
   Real x;
   int lastIdx = -1;

   for (int i = this->thesolver->dim() - 1; i >= 0; --i)
   {
      x = fTest[i];

      if (x < -tol)
      {
         x = computePrice(x, coWeights_ptr[i], tol);

         if (x > best)
         {
            best = x;
            lastIdx = i;
         }
      }
   }

   return lastIdx;
}

  template <class R>
int SPxSteepPR<R>::selectLeaveSparse(Real tol)
{
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();
   const Real* fTest         = this->thesolver->fTest().get_const_ptr();
   Real best = -infinity;
   Real x;
   int lastIdx = -1;
   int idx;

   for (int i = this->thesolver->infeasibilities.size() - 1; i >= 0; --i)
   {
      idx = this->thesolver->infeasibilities.index(i);
      x = fTest[idx];

      if (x < -tol)
      {
         x = computePrice(x, coWeights_ptr[idx], tol);

         if (x > best)
         {
            best = x;
            lastIdx = idx;
         }
      }
      else
      {
         this->thesolver->infeasibilities.remove(i);
         assert(this->thesolver->isInfeasible[idx] == this->VIOLATED || this->thesolver->isInfeasible[idx] == this->VIOLATED_AND_CHECKED);
         this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
      }
   }

   return lastIdx;
}

  template <class R>
int SPxSteepPR<R>::selectLeaveHyper(Real tol)
{
   const Real* coPen = this->thesolver->coWeights.get_const_ptr();
   const Real* fTest = this->thesolver->fTest().get_const_ptr();
   Real leastBest = infinity;
   Real best = -infinity;
   Real x;
   int bestIdx = -1;
   int idx = 0;

   // find the best price from the short candidate list
   for( int i = bestPrices.size() - 1; i >= 0; --i )
   {
      idx = bestPrices.index(i);
      x = fTest[idx];
      if( x < -tol )
      {
         assert(this->thesolver->isInfeasible[idx] == this->VIOLATED || this->thesolver->isInfeasible[idx] == VIOLATED_AND_CHECKED);
         x = computePrice(x, coPen[idx], tol);

         if( x > best )
         {
            best = x;
            bestIdx = idx;
         }
         if( x < leastBest )
            leastBest = x;
      }
      else
      {
         bestPrices.remove(i);
         this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
      }
   }

   // make sure we do not skip potential candidates due to a high leastBest value
   if( leastBest == infinity )
   {
      assert(bestPrices.size() == 0);
      leastBest = 0;
   }

   // scan the updated indices for a better price
   for( int i = this->thesolver->updateViols.size() - 1; i >= 0; --i )
   {
      idx = this->thesolver->updateViols.index(i);
      // is this index a candidate for bestPrices?
      if( this->thesolver->isInfeasible[idx] == this->VIOLATED )
      {
         x = fTest[idx];
         assert(x < -tol);
         x = computePrice(x, coPen[idx], tol);

         if( x > leastBest )
         {
            if( x > best )
            {
               best = x;
               bestIdx = idx;
            }
            this->thesolver->isInfeasible[idx] = VIOLATED_AND_CHECKED;
            bestPrices.addIdx(idx);
         }
      }
   }

   return bestIdx;
}

/* Entering Simplex
 */
  template <class R>
void SPxSteepPR<R>::entered4(SPxId /* id */, int n)
{
   assert(this->thesolver->type() == SPxSolver<R>::ENTER);

   if (n >= 0 && n < this->thesolver->dim())
   {
      Real delta = 2 + 1.0 / this->thesolver->basis().iteration();
      Real* coWeights_ptr = this->thesolver->coWeights.get_ptr();
      Real* weights_ptr = this->thesolver->weights.get_ptr();
      const Real* workVec_ptr = workVec.get_const_ptr();
      const Real* pVec = this->thesolver->pVec().delta().values();
      const IdxSet& pIdx = this->thesolver->pVec().idx();
      const Real* coPvec = this->thesolver->coPvec().delta().values();
      const IdxSet& coPidx = this->thesolver->coPvec().idx();
      Real xi_p = 1 / this->thesolver->fVec().delta()[n];
      int i, j;
      Real xi_ip;

      assert(this->thesolver->fVec().delta()[n] > this->thesolver->epsilon()
              || this->thesolver->fVec().delta()[n] < -this->thesolver->epsilon());

      for (j = coPidx.size() - 1; j >= 0; --j)
      {
         i = coPidx.index(j);
         xi_ip = xi_p * coPvec[i];
         coWeights_ptr[i] += xi_ip * (xi_ip * pi_p - 2.0 * workVec_ptr[i]);
         /*
         if(coWeights_ptr[i] < 1)
             coWeights_ptr[i] = 1 / (2-x);
         */
         if (coWeights_ptr[i] < delta)
            coWeights_ptr[i] = delta;
         // coWeights_ptr[i] = 1;
         else if (coWeights_ptr[i] > infinity)
            coWeights_ptr[i] = 1 / this->thesolver->epsilon();
      }

      for (j = pIdx.size() - 1; j >= 0; --j)
      {
         i = pIdx.index(j);
         xi_ip = xi_p * pVec[i];
         weights_ptr[i] += xi_ip * (xi_ip * pi_p - 2.0 * (this->thesolver->vector(i) * workVec));
         /*
         if(weights_ptr[i] < 1)
             weights_ptr[i] = 1 / (2-x);
         */
         if (weights_ptr[i] < delta)
            weights_ptr[i] = delta;
         // weights_ptr[i] = 1;
         else if (weights_ptr[i] > infinity)
            weights_ptr[i] = 1.0 / this->thesolver->epsilon();
      }
   }

   /*@
       if(this->thesolver->isId(id))
           weights[   this->thesolver->number(id) ] *= 1.0001;
       else if(this->thesolver->isCoId(id))
           coWeights[ this->thesolver->number(id) ] *= 1.0001;
   */

}


  template <class R>
SPxId SPxSteepPR<R>::buildBestPriceVectorEnterDim( Real& best, Real feastol )
{
   const Real* coTest        = this->thesolver->coTest().get_const_ptr();
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();
   int idx;
   int nsorted;
   Real x;
   IdxElement price;

   prices.clear();
   bestPrices.clear();

   // construct vector of all prices
   for( int i = this->thesolver->infeasibilities.size() - 1; i >= 0; --i )
   {
      idx = this->thesolver->infeasibilities.index(i);
      x = coTest[idx];
      if ( x < -feastol)
      {
         // it might happen that we call the pricer with a tighter tolerance than what was used when computing the violations
         this->thesolver->isInfeasible[idx] = this->VIOLATED;
         price.val = computePrice(x, coWeights_ptr[idx], feastol);
         price.idx = idx;
         prices.append(price);
      }
      else
      {
         this->thesolver->infeasibilities.remove(i);
         this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
      }
   }
   // set up structures for the quicksort implementation
   compare.elements = prices.get_const_ptr();
   // do a partial sort to move the best ones to the front
   // TODO this can be done more efficiently, since we only need the indices
   nsorted = SPxQuicksortPart(prices.get_ptr(), compare, 0, prices.size(), HYPERPRICINGSIZE);
   // copy indices of best values to bestPrices
   for( int i = 0; i < nsorted; ++i )
   {
      bestPrices.addIdx(prices[i].idx);
      this->thesolver->isInfeasible[prices[i].idx] = this->VIOLATED_AND_CHECKED;
   }

   if( nsorted > 0 )
   {
      best = prices[0].val;
      return this->thesolver->coId(prices[0].idx);
   }
   else
      return SPxId();
}


  template <class R>
SPxId SPxSteepPR<R>::buildBestPriceVectorEnterCoDim( Real& best, Real feastol )
{
   const Real* test        = this->thesolver->test().get_const_ptr();
   const Real* weights_ptr = this->thesolver->weights.get_const_ptr();
   int idx;
   int nsorted;
   Real x;
   IdxElement price;

   pricesCo.clear();
   bestPricesCo.clear();

   // construct vector of all prices
   for( int i = this->thesolver->infeasibilitiesCo.size() - 1; i >= 0; --i )
   {
      idx = this->thesolver->infeasibilitiesCo.index(i);
      x = test[idx];
      if ( x < -feastol)
      {
         // it might happen that we call the pricer with a tighter tolerance than what was used when computing the violations
         this->thesolver->isInfeasibleCo[idx] = this->VIOLATED;
         price.val = computePrice(x, weights_ptr[idx], feastol);
         price.idx = idx;
         pricesCo.append(price);
      }
      else
      {
         this->thesolver->infeasibilitiesCo.remove(i);
         this->thesolver->isInfeasibleCo[idx] = NOT_VIOLATED;
      }
   }
   // set up structures for the quicksort implementation
   compare.elements = pricesCo.get_const_ptr();
   // do a partial sort to move the best ones to the front
   // TODO this can be done more efficiently, since we only need the indices
   nsorted = SPxQuicksortPart(pricesCo.get_ptr(), compare, 0, pricesCo.size(), HYPERPRICINGSIZE);
   // copy indices of best values to bestPrices
   for( int i = 0; i < nsorted; ++i )
   {
      bestPricesCo.addIdx(pricesCo[i].idx);
      this->thesolver->isInfeasibleCo[pricesCo[i].idx] = this->VIOLATED_AND_CHECKED;
   }

   if( nsorted > 0 )
   {
      best = pricesCo[0].val;
      return this->thesolver->id(pricesCo[0].idx);
   }
   else
      return SPxId();
}


  template <class R>
SPxId SPxSteepPR<R>::selectEnter()
{
   assert(thesolver != 0);
   SPxId enterId;

   enterId = selectEnterX(this->theeps);

   if( !enterId.isValid() && !refined )
   {
      refined = true;
      MSG_INFO3( (*this->thesolver->spxout), (*this->thesolver->spxout) << "WSTEEP05 trying refinement step..\n"; )
      enterId = selectEnterX(this->theeps/STEEP_REFINETOL);
   }

   assert(isConsistent());

   if (enterId.isValid())
   {
      SSVector& delta = this->thesolver->fVec().delta();

      this->thesolver->basis().solve4update(delta, this->thesolver->vector(enterId));

      workRhs.setup_and_assign(delta);
      pi_p = 1 + delta.length2();

      this->thesolver->setup4coSolve(&workVec, &workRhs);
   }
   return enterId;
}

  template <class R>
SPxId SPxSteepPR<R>::selectEnterX(Real tol)
{
   SPxId enterId;
   SPxId enterCoId;
   Real best;
   Real bestCo;

   best = -infinity;
   bestCo = -infinity;

   if( this->thesolver->hyperPricingEnter && !refined )
   {
      if( bestPrices.size() < 2 || this->thesolver->basis().lastUpdate() == 0 )
         enterCoId = (this->thesolver->sparsePricingEnter) ? buildBestPriceVectorEnterDim(best, tol) : selectEnterDenseDim(best, tol);
      else
         enterCoId = (this->thesolver->sparsePricingEnter) ? selectEnterHyperDim(best, tol) : selectEnterDenseDim(best, tol);

      if( bestPricesCo.size() < 2 || this->thesolver->basis().lastUpdate() == 0 )
         enterId = (this->thesolver->sparsePricingEnterCo) ? buildBestPriceVectorEnterCoDim(bestCo, tol) : selectEnterDenseCoDim(bestCo, tol);
      else
         enterId = (this->thesolver->sparsePricingEnterCo) ? selectEnterHyperCoDim(bestCo, tol) : selectEnterDenseCoDim(bestCo, tol);
   }
   else
   {
      enterCoId = (this->thesolver->sparsePricingEnter && !refined) ? selectEnterSparseDim(best, tol) : selectEnterDenseDim(best, tol);
      enterId = (this->thesolver->sparsePricingEnterCo && !refined) ? selectEnterSparseCoDim(bestCo, tol) : selectEnterDenseCoDim(bestCo, tol);
   }

   // prefer slack indices to reduce nonzeros in basis matrix
   if( enterCoId.isValid() && (best > SPARSITY_TRADEOFF * bestCo || !enterId.isValid()) )
      return enterCoId;
   else
      return enterId;
}


  template <class R>
SPxId SPxSteepPR<R>::selectEnterHyperDim(Real& best, Real tol)
{
   const Real* coTest        = this->thesolver->coTest().get_const_ptr();
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();

   Real leastBest = infinity;
   Real x;
   int enterIdx = -1;
   int idx;

   // find the best price from short candidate list
   for( int i = bestPrices.size() - 1; i >= 0; --i )
   {
      idx = bestPrices.index(i);
      x = coTest[idx];
      if( x < -tol )
      {
         x = computePrice(x, coWeights_ptr[idx], tol);
         if( x > best )
         {
            best = x;
            enterIdx = idx;
         }
         if( x < leastBest )
            leastBest = x;
      }
      else
      {
         bestPrices.remove(i);
         this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
      }
   }

   // make sure we do not skip potential candidates due to a high leastBest value
   if( leastBest == infinity )
   {
      assert(bestPrices.size() == 0);
      leastBest = 0;
   }

   // scan the updated indices for a better price
   for( int i = this->thesolver->updateViols.size() -1; i >= 0; --i )
   {
      idx = this->thesolver->updateViols.index(i);
      // only look at indices that were not checked already
      if( this->thesolver->isInfeasible[idx] == this->VIOLATED )
      {
         x = coTest[idx];
         if( x < -tol )
         {
            x = computePrice(x, coWeights_ptr[idx], tol);
            if( x > leastBest )
            {
               if (x > best)
               {
                  best = x;
                  enterIdx = idx;
               }
               // put index into candidate list
               this->thesolver->isInfeasible[idx] = VIOLATED_AND_CHECKED;
               bestPrices.addIdx(idx);
            }
         }
         else
         {
            this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
         }
      }
   }

   if( enterIdx >= 0 )
      return this->thesolver->coId(enterIdx);
   else
      return SPxId();
}


  template <class R>
SPxId SPxSteepPR<R>::selectEnterHyperCoDim(Real& best, Real tol)
{
   const Real* test        = this->thesolver->test().get_const_ptr();
   const Real* weights_ptr = this->thesolver->weights.get_const_ptr();

   Real leastBest = infinity;
   Real x;
   int enterIdx = -1;
   int idx;

   // find the best price from short candidate list
   for( int i = bestPricesCo.size() - 1; i >= 0; --i )
   {
      idx = bestPricesCo.index(i);
      x = test[idx];
      if( x < -tol )
      {
         x = computePrice(x, weights_ptr[idx], tol);
         if( x > best )
         {
            best = x;
            enterIdx = idx;
         }
         if( x < leastBest )
            leastBest = x;
      }
      else
      {
         bestPricesCo.remove(i);
         this->thesolver->isInfeasibleCo[idx] = NOT_VIOLATED;
      }
   }

   // make sure we do not skip potential candidates due to a high leastBest value
   if( leastBest == infinity )
   {
      assert(bestPricesCo.size() == 0);
      leastBest = 0;
   }

   // scan the updated indices for a better price
   for( int i = this->thesolver->updateViolsCo.size() -1; i >= 0; --i )
   {
      idx = this->thesolver->updateViolsCo.index(i);
      // only look at indices that were not checked already
      if( this->thesolver->isInfeasibleCo[idx] == this->VIOLATED )
      {
         x = test[idx];
         if( x < -tol )
         {
            x = computePrice(x, weights_ptr[idx], tol);
            if( x > leastBest )
            {
               if (x > best)
               {
                  best = x;
                  enterIdx = idx;
               }
               // put index into candidate list
               this->thesolver->isInfeasibleCo[idx] = VIOLATED_AND_CHECKED;
               bestPricesCo.addIdx(idx);
            }
         }
         else
         {
            this->thesolver->isInfeasibleCo[idx] = NOT_VIOLATED;
         }
      }
   }

   if( enterIdx >= 0 )
      return this->thesolver->id(enterIdx);
   else
      return SPxId();
}


  template <class R>
SPxId SPxSteepPR<R>::selectEnterSparseDim(Real& best, Real tol)
{
   SPxId enterId;
   const Real* coTest        = this->thesolver->coTest().get_const_ptr();
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();

   int idx;
   Real x;

   for (int i = this->thesolver->infeasibilities.size() -1; i >= 0; --i)
   {
      idx = this->thesolver->infeasibilities.index(i);
      x = coTest[idx];

      if (x < -tol)
      {
         x = computePrice(x, coWeights_ptr[idx], tol);
         if (x > best)
         {
            best = x;
            enterId = this->thesolver->coId(idx);
         }
      }
      else
      {
         this->thesolver->infeasibilities.remove(i);
         this->thesolver->isInfeasible[idx] = NOT_VIOLATED;
      }
   }
   return enterId;
}

  template <class R>
SPxId SPxSteepPR<R>::selectEnterSparseCoDim(Real& best, Real tol)
{
   SPxId enterId;
   const Real* test          = this->thesolver->test().get_const_ptr();
   const Real* weights_ptr   = this->thesolver->weights.get_const_ptr();

   int idx;
   Real x;

   for (int i = this->thesolver->infeasibilitiesCo.size() -1; i >= 0; --i)
   {
      idx = this->thesolver->infeasibilitiesCo.index(i);
      x = test[idx];

      if (x < -tol)
      {
         x = computePrice(x, weights_ptr[idx], tol);
         if (x > best)
         {
            best   = x;
            enterId = this->thesolver->id(idx);
         }
      }
      else
      {
         this->thesolver->infeasibilitiesCo.remove(i);
         this->thesolver->isInfeasibleCo[idx] = NOT_VIOLATED;
      }
   }
   return enterId;
}

  template <class R>
SPxId SPxSteepPR<R>::selectEnterDenseDim(Real& best, Real tol)
{
   SPxId enterId;
   const Real* coTest        = this->thesolver->coTest().get_const_ptr();
   const Real* coWeights_ptr = this->thesolver->coWeights.get_const_ptr();

   Real x;

   for (int i = 0, end = this->thesolver->dim(); i < end; ++i)
   {
      x = coTest[i];
      if (x < -tol)
      {
         x = computePrice(x, coWeights_ptr[i], tol);
         if (x > best)
         {
            best = x;
            enterId = this->thesolver->coId(i);
         }
      }
   }
   return enterId;
}

  template <class R>
SPxId SPxSteepPR<R>::selectEnterDenseCoDim(Real& best, Real tol)
{
   SPxId enterId;
   const Real* test          = this->thesolver->test().get_const_ptr();
   const Real* weights_ptr   = this->thesolver->weights.get_const_ptr();

   Real x;

   for(int i = 0, end = this->thesolver->coDim(); i < end; ++i)
   {
      x = test[i];
      if (x < -tol)
      {
         x = computePrice(x, weights_ptr[i], tol);
         if (x > best)
         {
            best   = x;
            enterId = this->thesolver->id(i);
         }
      }
   }
   return enterId;
}


  template <class R>
void SPxSteepPR<R>::addedVecs(int n)
{
   DVector& weights = this->thesolver->weights;
   n = weights.dim();
   weights.reDim(this->thesolver->coDim());

   if (this->thesolver->type() == SPxSolver<R>::ENTER)
   {
      for (; n < weights.dim(); ++n)
         weights[n] = 2;
   }
}

  template <class R>
void SPxSteepPR<R>::addedCoVecs(int n)
{
   DVector& coWeights = this->thesolver->coWeights;
   n = coWeights.dim();
   workVec.reDim (this->thesolver->dim());
   coWeights.reDim (this->thesolver->dim());
   for (; n < coWeights.dim(); ++n)
      coWeights[n] = 1;
}

  template <class R>
void SPxSteepPR<R>::removedVec(int i)
{
   assert(thesolver != 0);
   DVector& weights = this->thesolver->weights;
   weights[i] = weights[weights.dim()];
   weights.reDim(this->thesolver->coDim());
}

  template <class R>
void SPxSteepPR<R>::removedVecs(const int perm[])
{
   assert(thesolver != 0);
   DVector& weights = this->thesolver->weights;
   if (this->thesolver->type() == SPxSolver<R>::ENTER)
   {
      int i;
      int j = weights.dim();
      for (i = 0; i < j; ++i)
      {
         if (perm[i] >= 0)
            weights[perm[i]] = weights[i];
      }
   }
   weights.reDim(this->thesolver->coDim());
}

  template <class R>
void SPxSteepPR<R>::removedCoVec(int i)
{
   assert(thesolver != 0);
   DVector& coWeights = this->thesolver->coWeights;
   coWeights[i] = coWeights[coWeights.dim()];
   coWeights.reDim(this->thesolver->dim());
}

  template <class R>
void SPxSteepPR<R>::removedCoVecs(const int perm[])
{
   assert(thesolver != 0);
   DVector& coWeights = this->thesolver->coWeights;
   int i;
   int j = coWeights.dim();
   for (i = 0; i < j; ++i)
   {
      if (perm[i] >= 0)
         coWeights[perm[i]] = coWeights[i];
   }
   coWeights.reDim(this->thesolver->dim());
}

  template <class R>
bool SPxSteepPR<R>::isConsistent() const
{
#ifdef ENABLE_CONSISTENCY_CHECKS
   DVector& w = this->thesolver->weights;
   DVector& coW = this->thesolver->coWeights;
   if (thesolver != 0 && this->thesolver->type() == SPxSolver<R>::LEAVE && setup == EXACT)
   {
      int i;
      SSVector tmp(this->thesolver->dim(), this->thesolver->epsilon());
      Real x;
      for (i = this->thesolver->dim() - 1; i >= 0; --i)
      {
         this->thesolver->basis().coSolve(tmp, this->thesolver->unitVector(i));
         x = coW[i] - tmp.length2();
         if (x > this->thesolver->leavetol() || -x > this->thesolver->leavetol())
         {
            MSG_ERROR( std::cerr << "ESTEEP03 x[" << i << "] = " << x << std::endl; )
         }
      }
   }

   if (thesolver != 0 && this->thesolver->type() == SPxSolver<R>::ENTER)
   {
      int i;
      for (i = this->thesolver->dim() - 1; i >= 0; --i)
      {
         if (coW[i] < this->thesolver->epsilon())
            return MSGinconsistent("SPxSteepPR");
      }

      for (i = this->thesolver->coDim() - 1; i >= 0; --i)
      {
         if (w[i] < this->thesolver->epsilon())
            return MSGinconsistent("SPxSteepPR");
      }
   }
#endif

   return true;
}
} // namespace soplex
