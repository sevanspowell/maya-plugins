#pragma once

#include <maya/MPxContextCommand.h>

#include "DrawCurveContext.h"

class DrawCurveContextCommand : public MPxContextCommand {
  public:
    DrawCurveContextCommand() : m_context(nullptr){};
    virtual ~DrawCurveContextCommand(){};
    static void *creator();

    virtual MStatus doEditFlags() override;
    virtual MStatus doQueryFlags() override;
    virtual MStatus appendSyntax() override;
    virtual MPxContext *makeObj() override;

    constexpr static const char *const LENGTH_FLAG      = "-l" ;
    constexpr static const char *const LENGTH_FLAG_LONG = "-length";

  protected:
    DrawCurveContext *m_context;
};
