/*
 * Copyright (c) 2000-2004
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
package conflux.flowgraph;

import soot.*;

/**
 * Class representing an entire flow graph
 *
 * @author John Kodumal
 */
public class FlowGraph implements PointsToAnalysis {



    // TODO PointsToAnalysis interface

    /** Returns the set of objects pointed to by variable l. */
    // TODO
    public PointsToSet reachingObjects( Local l ) {
	return null;
    }

    /** Returns the set of objects pointed to by variable l in context c. */
    // TODO
    public PointsToSet reachingObjects( Context c, Local l ) {
	return null;
    }

    /** Returns the set of objects pointed to by static field f. */
    // TODO
    public PointsToSet reachingObjects( SootField f ) {
	return null;
    }

    /** Returns the set of objects pointed to by instance field f
     * of the objects in the PointsToSet s. */
    // TODO
    public PointsToSet reachingObjects( PointsToSet s, SootField f ) {
	return null;
    }

    /** Returns the set of objects pointed to by instance field f
     * of the objects pointed to by l. */
    // TODO
    public PointsToSet reachingObjects( Local l, SootField f ) {
	return null;
    }

    /** Returns the set of objects pointed to by instance field f
     * of the objects pointed to by l in context c. */
    // TODO
    public PointsToSet reachingObjects( Context c, Local l, SootField f ) {
	return null;
    }

    /** Returns the set of objects pointed to by elements of the arrays
     * in the PointsToSet s. */
    // TODO
    public PointsToSet reachingObjectsOfArrayElement( PointsToSet s ) {
	return null;
    }


}