function parse(node)
    if node.kind == 0 then
        writeln(".global " .. node.identifier)
        writeln(node.identifier .. ":")
        return 0
    end
end
