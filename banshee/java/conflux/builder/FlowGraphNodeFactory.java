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
package conflux.builder;

import conflux.flowgraph.*;
import soot.jimple.*;
import soot.*;

/** Builds flow graph nodes for each Soot value
 *
 * @author John Kodumal
 */
public class FlowGraphNodeFactory extends AbstractJimpleValueSwitch {
    FlowGraph fg;
    MethodFlowGraph mfg;

    public FlowGraphNodeFactory(FlowGraph fg, MethodFlowGraph mfg) {
	this.fg = fg;
	this.mfg = mfg;
    }

    // TODO
    public final void handleStmt(Stmt s) {
	if (s.containsInvokeExpr()) {
	    return;
	}
	s.apply( new AbstractStmtSwitch() {
		public final void caseAssignStmt(AssignStmt as) {
		    Value l = as.getLeftOp();
		    Value r = as.getRightOp();
		    if ( !( l.getType() instanceof RefLikeType)) return;
		    l.apply(FlowGraphNodeFactory.this);
		    FlowGraphNode dest = getFlowGraphNode();
		    r.apply(FlowGraphNodeFactory.this);
		    FlowGraphNode src = getFlowGraphNode();
		    mfg.addSubtypeEdge(src, dest);
		}
	    } );		
    }

    public final FlowGraphNode getFlowGraphNode() {
	return (FlowGraphNode) getResult();
    }

}

