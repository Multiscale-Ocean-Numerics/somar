#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include <iostream>
#include <algorithm>
#include "BoxLayout.H"
#include "DataIterator.H"
#include "TimedDataIterator.H"
#include "LayoutIterator.H"
#include "SPMD.H"
#include "parstream.H"
#include "CH_Timer.H"
#include "NamespaceHeader.H"

using std::ostream;


// -----------------------------------------------------------------------------
BoxLayout::BoxLayout()
: m_boxes(new Vector<Entry>())
, m_layout(new int)
, m_closed(new bool(false))
, m_sorted(new bool(false))
, m_dataIterator()
, m_indicies(new Vector<LayoutIndex>())
{
}


// -----------------------------------------------------------------------------
BoxLayout::BoxLayout(const Vector<Box>& a_boxes, const Vector<int>& assignments)
  :m_boxes( new Vector<Entry>()),
   m_layout(new int),
   m_closed(new bool(false)),
   m_sorted(new bool(false)),
   m_indicies(new Vector<LayoutIndex>())
{
  define(a_boxes, assignments);
}


// -----------------------------------------------------------------------------
BoxLayout::~BoxLayout()
{
}


// -----------------------------------------------------------------------------
void
BoxLayout::define(const Vector<Box>& a_boxes, const Vector<int>& a_procIDs)
{
    checkDefine(a_boxes, a_procIDs);
    const unsigned int num_boxes = a_boxes.size();
    // const int num_procs = a_procIDs.size();
    m_boxes->resize(num_boxes);
    for (unsigned int i = 0; i < num_boxes; ++i) {
        m_boxes->operator[](i) = a_boxes[i];
        if (numProc() > 1) {
            m_boxes->operator[](i).m_procID = a_procIDs[i];
        } else {
            m_boxes->operator[](i).m_procID = 0;
        }
    }
    close();
}

// -----------------------------------------------------------------------------
BoxLayout::BoxLayout(const BoxLayout &a_rhs)
: m_boxes(a_rhs.m_boxes)
, m_layout(a_rhs.m_layout)
, m_closed(a_rhs.m_closed)
, m_sorted(a_rhs.m_sorted)
, m_dataIterator(a_rhs.m_dataIterator)
#ifdef CH_MPI
, m_dataIndex(a_rhs.m_dataIndex)
#endif
{
}

// -----------------------------------------------------------------------------
BoxLayout&
BoxLayout::operator=(const BoxLayout& a_rhs)
{
    if (this == &a_rhs) return *this;

    m_boxes        = a_rhs.m_boxes;
    m_layout       = a_rhs.m_layout;
    m_closed       = a_rhs.m_closed;
    m_sorted       = a_rhs.m_sorted;
    m_dataIterator = a_rhs.m_dataIterator;
#ifdef CH_MPI
    m_dataIndex = a_rhs.m_dataIndex;
#endif
    return *this;
}


// -----------------------------------------------------------------------------
void
BoxLayout::
transform(BaseTransform& a_transform)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      Box fullBox = (*m_boxes)[ivec].box;
      (*m_boxes)[ivec].box = a_transform(fullBox);
    }
}


// -----------------------------------------------------------------------------
//need at least one non-inlined function, otherwise
//some compilers don't build a class description in the
//object file.
Box BoxLayout::get(const LayoutIterator& it) const
{
  return get(it());
}
Box BoxLayout::get(const DataIterator& it) const
{
  return get(it());
}

Box
BoxLayout::operator[](const LayoutIterator& index) const
{
  return this->operator[](index());
}

Box
BoxLayout::operator[](const DataIterator& index) const
{
  return this->operator[](index());
}

void BoxLayout::sort()
{
  if (!*m_closed)
    {
      std::sort(m_boxes->begin(), m_boxes->end());
      *m_sorted = true;
    }
}

void BoxLayout::closeNoSort()
{
  if (!*m_closed)
    {
      //sort();  there, no sort.   The sort is a lie.
      *m_sorted = false;
      *m_closed = true;
      buildDataIndex();
      m_dataIterator.reset(new DataIterator(*this, m_layout.get()));
    }
}

void BoxLayout::close()
{
  if (!*m_closed)
    {
      sort();
      *m_closed = true;
      buildDataIndex();
      m_dataIterator.reset(new DataIterator(*this, m_layout.get()));
    }
}

void BoxLayout::buildDataIndex()
{
#ifdef CH_MPI
  std::list<DataIndex> dlist;
  unsigned int index = 0;
  unsigned int datIn = 0;
  unsigned int p = CHprocID();
  int count=0;
  const Entry* box;

  while (index < size())
    {
      box = &(*(m_boxes))[index];
      if (box->m_procID == p)
        {
          DataIndex current(index, datIn, m_layout.get());
          dlist.push_back(current);
          count++;
          datIn++;
        }
      ++index;
    }

  m_dataIndex.reset(new Vector<DataIndex>(count));
  std::list<DataIndex>::iterator b=dlist.begin();
  for (int i=0; i<count; ++i, ++b)
    {
      m_dataIndex->operator[](i) = *b;
    }
#endif
}

bool BoxLayout::coarsenable(int refRatio) const
{
 // if (size() == 0) return false;
  for (unsigned int i=0; i<size(); i++)
    {
      Box b =  m_boxes->operator[](i).box;
      b.coarsen(refRatio);
      b.refine(refRatio);
      if (b !=  m_boxes->operator[](i).box)
        return false;
    }
  return true;
}
zipDataIterator<BoxLayout> BoxLayout::begin() {return zipDataIterator<BoxLayout>(*this);}

zipDataIterator<const BoxLayout> BoxLayout::begin() const {return zipDataIterator<const BoxLayout>(*this);}

DataIterator BoxLayout::dataIterator() const
{
  CH_assert(*m_closed);
  //DataIterator rtn(*this, m_layout);
  return *m_dataIterator;
  //return rtn;
}

TimedDataIterator BoxLayout::timedDataIterator() const
{
  CH_assert(*m_closed);
  TimedDataIterator rtn(*this, m_layout.get());
  return rtn;
}

LayoutIterator BoxLayout::layoutIterator() const
{
  return LayoutIterator(*this, m_layout.get());
}

void
BoxLayout::checkDefine(const Vector<Box>& a_boxes, const Vector<int>& a_procIDs)
{
    if (*m_closed) {
        MayDay::Error("attempt to define(..) a closed BoxLayout");
    }
    const unsigned int numBoxes = a_boxes.size();
    const unsigned int numProcs = a_procIDs.size();
    if ((numProc() > 1) && (numBoxes != numProcs)) {
        MayDay::Error(
            "BoxLayout::define(): vector of processor assignments is different "
            "length from vector of boxes");
    }
    // Check for negative proc ID's and ID's larger than total number of procs.
    for (unsigned int i = 0; i < numProcs; ++i) {
        if (a_procIDs[i] < 0) {
            MayDay::Error(
                "BoxLayout::define(): Negative processor assignments not "
                "allowed");
        }
        //    if (a_procIDs[i] >= numProc())
        //         {
        //           MayDay::Error("BoxLayout::define(): Attempting to assign
        //           data to processor ID larger than total number of processors
        //           available");
        //         }
    }
}


// Other member functions
// ======================

void
BoxLayout::deepCopy(const BoxLayout& a_source)
{
  m_boxes.reset(new Vector<Entry>(*(a_source.m_boxes)));
  m_layout = a_source.m_layout;
#ifdef CH_MPI
  m_dataIndex = a_source.m_dataIndex;
#endif
  *m_closed = false;
}



//checks equality of the vector of boxes inside m_boxes
bool BoxLayout::sameBoxes(const BoxLayout& a_layout) const
{
  bool retval;
  if (size() == a_layout.size())
    {
      retval = true;

      for (unsigned int iBox = 0; iBox < size(); ++iBox)
        {
          //RefCountedPtr<Vector<Entry> > m_boxes;
          if ((*m_boxes)[iBox].box != (*a_layout.m_boxes)[iBox].box)
            {
              retval = false;
            }
        }
    }
  else
    {
      retval = false;
    }
  return retval;
}

// Global functions
// ================

// For now, we can just have the one coarsen funtion.  If a DisjointBoxLayout
// enters this function, is coarsened, and then doesn't remain disjoint, it
// will be caught here at the call to close().  Debugging should not be

void
coarsen(BoxLayout& a_output, const BoxLayout& a_input, int a_refinement)
{
   if (!a_input.isClosed())
    {
      MayDay::Error("input to coarsen must be called with closed BoxLayout");
    }
  if (a_output.isClosed())
    {
      MayDay::Error("output of coarsen must be called on open BoxLayout");
    }
  //a_output.deepCopy(a_input);
  a_output.m_boxes.reset(new Vector<Entry>(*(a_input.m_boxes)));
  a_output.m_layout     = a_input.m_layout;
#ifdef CH_MPI
  a_output.m_dataIndex  = a_input.m_dataIndex;
#endif

  for (unsigned int ivec = 0; ivec < a_output.m_boxes->size(); ivec++)
    {
      (*a_output.m_boxes)[ivec].box.coarsen(a_refinement);
    }
  a_output.close();
}

void
BoxLayout::
operator&= (const Box& a_box)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box &= a_box;
    }
}


void
BoxLayout::
operator&= (const ProblemDomain& a_domain)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box &= a_domain;
    }
}


void
BoxLayout::
adjCellSide(int a_idir, int a_length, Side::LoHiSide a_side)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      Box fullBox = (*m_boxes)[ivec].box;
      if (a_side == Side::Lo)
        {
          (*m_boxes)[ivec].box = adjCellLo(fullBox, a_idir, a_length);
        }
      else
        {
          (*m_boxes)[ivec].box = adjCellHi( fullBox, a_idir, a_length);
        }
    }
}

void
BoxLayout::
growSide(int a_idir, int a_length, Side::LoHiSide a_side)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      if (a_side == Side::Lo)
        {
          (*m_boxes)[ivec].box.growLo(a_idir, a_length);
        }
      else
        {
          (*m_boxes)[ivec].box.growHi(a_idir, a_length);
        }
    }
}
//////////////
void
BoxLayout::
surroundingNodes()
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.surroundingNodes();
    }
}

//////////////
void
BoxLayout::
convertNewToOld(const IntVect& a_permutation,
                const IntVect& a_sign,
                const IntVect& a_translation)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.convertNewToOld(a_permutation, a_sign, a_translation);
    }
}
//////////////
void
BoxLayout::
convertOldToNew(const IntVect& a_permutation,
                const IntVect& a_sign,
                const IntVect& a_translation)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.convertOldToNew(a_permutation, a_sign, a_translation);
    }
}
///////////
void
BoxLayout::
enclosedCells()
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.enclosedCells();
    }
}

///////////
void
BoxLayout::
grow(int a_growth)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.grow(a_growth);
    }
}
///////////
void
BoxLayout::
grow(int a_idir, int a_growth)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.grow(a_idir, a_growth);
    }
}
///////////
void
BoxLayout::
grow(IntVect a_growth)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.grow(a_growth);
    }
}

///////////
void
BoxLayout::
coarsen(int a_ref)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.coarsen(a_ref);
    }
}
///////////
void
BoxLayout::
refine(int a_ref)
{
  for (unsigned int ivec = 0; ivec < m_boxes->size(); ivec++)
    {
      (*m_boxes)[ivec].box.refine(a_ref);
    }
}

// we have an easier time with refine, since we know that refinement will
// not change the state of a sort, but, we will play it safe for now
// until this function shows up in the profiler.

void refine(BoxLayout& a_output, const BoxLayout& a_input, int a_refinement)
{
  if (!a_input.isClosed())
    {
      MayDay::Error("input to refine must be called with closed BoxLayout");
    }
  if (a_output.isClosed())
    {
      MayDay::Error("output of refine must be called on open BoxLayout");
    }
  a_output.deepCopy(a_input);

  for (unsigned int ivec = 0; ivec < a_output.m_boxes->size(); ivec++)
    {
      (*a_output.m_boxes)[ivec].box.refine(a_refinement);
    }
  a_output.close();
}

ostream& operator<<(ostream& os, const BoxLayout& a_layout)
{
  int i=0;
  for (LayoutIterator it(a_layout.layoutIterator()); it.ok(); ++it)
    {
      os << a_layout.get(it())<<"["<<a_layout.procID(it())<<"]";
      ++i;
      if (i==4)
      {
        os <<"\n"; i=0;
      }
      else
      {
        os <<" # ";
      }
    }

  os <<"\n";
  return os;
}
#ifdef CH_USE_PYTHON
BoxLayout::BoxLayout(PyObject * a_pin)
:m_boxes( new Vector<Entry>()),
   m_layout(new int),
   m_closed(new bool(false)),
   m_sorted(new bool(false)),
   m_indicies(new Vector<LayoutIndex>())
{
  int n=static_cast<int>(PyLong_AsLong(PyTuple_GetItem(a_pin,0)));
  Vector<Box> boxes(n);
  Vector<int> PIDs(n);
  for (auto i = 1; i<n+1; ++i)
  {
    boxes[i-1]=Box(PyTuple_GetItem(a_pin,2*i-1));
    PIDs[i-1]=static_cast<size_t>(PyLong_AsLong(PyTuple_GetItem(a_pin,2*i)));
  }
  define(boxes, PIDs);
}

PyObject *BoxLayout::pack() const
{

  PyObject *pArg = PyTuple_New(2+1);
  PyObject *pBoxes=PyTuple_New(this->size());
  PyObject *pIds=PyTuple_New(this->size());
  int i=0;
  for (auto LI=this->layoutIterator(); LI.ok(); ++LI, ++i)
  {

    Box box=this->get(LI());
    PyObject *pBox=box.pack();

    PyObject *pId= PyTuple_New(2);
    PyTuple_SetItem(pId, 0, PyLong_FromLong((int)this->procID(LI())));
    PyTuple_SetItem(pId, 1, PyUnicode_FromString(std::string("int").c_str()));

    PyTuple_SetItem(pBoxes, i, pBox);
    PyTuple_SetItem(pIds, i,  pId);
  }


    PyTuple_SetItem(pArg, 0, pBoxes);
    PyTuple_SetItem(pArg, 1, pIds);
    std::string Label = "BoxLayout";
    PyObject *pLabel = PyUnicode_FromString(Label.c_str());
    PyTuple_SetItem(pArg, 2, pLabel);

  return pArg;
}
#endif
void BoxLayout::print() const
{
  std::cout << *this;
}

int BoxLayout::numBoxes(const int procID) const
{
  int num = 0;
  for (unsigned int i=0; i<m_boxes->size(); ++i)
    {
      if (m_boxes->operator[](i).m_procID == (unsigned int) procID) ++num;
    }
  return num;
}

long long
BoxLayout::numCells() const
{
    long long rtn = 0;
    for (const auto& entry : *m_boxes) {
        rtn += entry.box.numPts();
    }
    return rtn;
}

// This was inlined but the GNU compiler in optimized mode produced incorrect
// code (at least some of the time) which was VERY BAD!  It is believed that
// this is a problem due in inlining a STL function which is precompiled...
unsigned int
BoxLayout::size() const
{
  return m_boxes->size();
}

Vector<Box> BoxLayout::boxArray() const
{
  Vector<Box> result( m_boxes->size() );

  // FIXME: maybe we want the order to be the one LayoutIterator would take?
  for ( unsigned int i=0;i<m_boxes->size();++i )
  {
    result[i] = (*m_boxes)[i].box;
  }
  return result;
}

Vector<int> BoxLayout::procIDs() const
{
  Vector<int> result( m_boxes->size() );
  for ( unsigned int i=0;i<m_boxes->size();++i )
  {
    result[i] = (*m_boxes)[i].m_procID;
  }
  return result;
}

class MortonOrdering
{
public:
  MortonOrdering(int a_maxSize):maxSize(a_maxSize)
  {
  }

  MortonOrdering()
    :
    maxSize(8*sizeof(int)-2)
  {
  }

  inline bool operator()(const Box& lhs, const Box& rhs) const;

  int maxSize;
};

inline bool MortonOrdering::operator()(const Box& lhs, const Box& rhs) const
{
  const IntVect l = lhs.smallEnd();
  const IntVect r = rhs.smallEnd();
  for (int i = maxSize; i>0; i--)
    {
      const int N = (1<<i); // march from most significant bit to least.
      for (int dir=CH_SPACEDIM-1; dir>=0; dir--)
        {
          if      ((l[dir]/N) < (r[dir]/N)) return true;
          else if ((l[dir]/N) > (r[dir]/N)) return false;
        }
    }
  return false ;
}

int maxBits(std::vector<Box>::iterator a_first, std::vector<Box>::iterator a_last)
{
  int maxSize = 0;
  for (std::vector<Box>::iterator p= a_first; p<a_last; ++p)
    {
      IntVect small = p->smallEnd();
      D_EXPR6( maxSize = Max(maxSize, Abs(small[0])),
               maxSize = Max(maxSize, Abs(small[1])),
               maxSize = Max(maxSize, Abs(small[2])),
               maxSize = Max(maxSize, Abs(small[3])),
               maxSize = Max(maxSize, Abs(small[4])),
               maxSize = Max(maxSize, Abs(small[5])));
    }
  int bits;
  for (bits=8*sizeof(int)-2; bits>0; bits--)
    {
      const int N = (1<<bits);
      int rem = maxSize/N;
      if (rem > 0) break;
    }
  bits++;
  return bits;
}


#ifdef CH_MPI

void parallelMortonOrdering(std::vector<Box>::iterator a_first, std::vector<Box>::iterator a_last,
                            int& a_maxBits, MPI_Comm& comm)
{
  int procs = 0, rank=0;
  int size = a_last - a_first;
  const bool newversion = true;

  MPI_Comm_size ( comm, &procs );
  MPI_Comm_rank ( comm, &rank  );

  if (size < 2000 || procs == 1)
    {
      a_maxBits = maxBits(a_first, a_last);
      std::sort(a_first, a_last, MortonOrdering(a_maxBits));
    }
  else
    {
      MPI_Comm split_comm;
      int middleRank = procs/2;
      int color;
      std::vector<Box>::iterator first, last, middle = a_first + size/2;
      if ( newversion )
      {
          color = rank%2;
          if (color == 0)
          {
              first = a_first;
              last  = middle;
          }
          else
          {
              first = middle;
              last  = a_last;
          }
      }
      else
      {
          if (rank < middleRank)
          {
              color = 0;
              first = a_first;
              last  = middle;
          }
          else
          {
              color = 1;
              first = middle;
              last  = a_last;
          }
      }

      MPI_Comm_split(comm, color, rank, &split_comm);
      int maxBits;
      parallelMortonOrdering(first, last, maxBits, split_comm);

      MPI_Comm_free(&split_comm);

      int countLo = (middle - a_first )*sizeof(Box);
      int countHi = (a_last - middle )*sizeof(Box);
      MPI_Status status;

      if ( !newversion )
      {
        if (color == 0)
        {
            MPI_Send(&(*a_first), countLo, MPI_CHAR, rank+middleRank, 0, comm);
            MPI_Recv(&(*middle),countHi, MPI_CHAR, rank+middleRank, 0, comm, &status);
        }
        else
        {
            MPI_Recv(&(*a_first),  countLo, MPI_CHAR, rank-middleRank, 0, comm, &status);
            MPI_Send(&(*middle), countHi, MPI_CHAR, rank-middleRank, 0, comm);
        }
        // middle sends to end of ragged edge
        if (middleRank*2 != procs && rank == middleRank)
        {
            MPI_Send(&(*a_first),  countLo, MPI_CHAR, procs-1, 0, comm);
            MPI_Recv(&(*middle), countHi, MPI_CHAR, procs-1, 0, comm, &status);
        }
      }
      else
      {
        if (color == 0)
        {
          // last proc of ragged edge -- s/r back
          if (procs%2 != 0 && rank == procs-1)
          {
            MPI_Sendrecv(&(*a_first), countLo, MPI_CHAR, rank-1, 0,
                         &(*middle),  countHi, MPI_CHAR, rank-1, 0, comm, &status);
          }
          else
          {
            // normal s/r up
            MPI_Sendrecv(&(*a_first), countLo, MPI_CHAR, rank+1, 0,
                         &(*middle),  countHi, MPI_CHAR, rank+1, 0, comm, &status);
          }
        }
        else
        {
          // normal s/r back
          MPI_Sendrecv(&(*middle), countHi, MPI_CHAR, rank-1, 0,
                       &(*a_first),countLo, MPI_CHAR, rank-1, 0, comm, &status);
          // special r/s forward
          if (procs%2 != 0 && rank == procs-2)
          {
            MPI_Sendrecv(&(*middle), countHi, MPI_CHAR, rank+1, 0,
                         &(*a_first),countLo, MPI_CHAR, rank+1, 0, comm, &status);

          }
        }
      }
      MPI_Allreduce (&maxBits, &a_maxBits, 1, MPI_INT, MPI_MAX, comm );
      std::inplace_merge(a_first, middle, a_last, MortonOrdering(a_maxBits));
    }
}
#endif

// testing version that runs both the serial and parallel versions and compares.
// void mortonOrdering(Vector<Box>& a_boxes)
// {
//   int bits;
// #ifdef CH_MPI
//   Vector<Box> tmp(a_boxes);
//   std::vector<Box>& a = tmp.stdVector();
//   parallelMortonOrdering(a.begin(), a.end(), bits, Chombo_MPI::comm);
// #endif
//   std::vector<Box>& b = a_boxes.stdVector();

//   bits = maxBits(b.begin(), b.end());
//   std::sort(b.begin(), b.end(), MortonOrdering(bits));

// #ifdef CH_MPI
//   std::vector<Box>::iterator ita=a.begin(), itb=b.begin();
//   for (;itb<b.end(); ++ita, ++itb)
//     {
//       if (*ita != *itb) MayDay::Error("parallel Morton ordering failed");
//     }
// #endif
// }

void
mortonOrdering(Vector<Box>& a_boxes)
{
    CH_TIME("mortonOrdering");
    int bits;
#ifdef CH_MPI
    parallelMortonOrdering(
        a_boxes.begin(), a_boxes.end(), bits, Chombo_MPI::comm);
#else
    bits = maxBits(a_boxes.begin(), a_boxes.end());
    std::sort(a_boxes.begin(), a_boxes.end(), MortonOrdering(bits));
#endif
}

#include "NamespaceFooter.H"
